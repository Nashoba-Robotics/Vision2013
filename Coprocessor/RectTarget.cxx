#include <stdio.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "RectTarget.h"
#include "RectTargetMeasured.h"

using namespace cv;
using namespace std;

/**
   @brief Default contructor
*/
RectTarget::RectTarget() {
  centerX = 0;
  centerY = 0;
  sizeX = 0;
  sizeY = 0;
  aspectRatio = 0;
  distanceX = 0;
  distanceY = 0;
  angleX = 0;
  tension = 0;
  targetType = TARGET_HEIGHT_UNKNOWN;
  valid = 0;
}

/**
   @brief Return the type of target as a string
*/
const char *RectTarget::getTargetTypeString() {
  switch (targetType) {
  case TARGET_HEIGHT_HIGH: return "High";
  case TARGET_HEIGHT_MIDDLE: return "Middle";
  case TARGET_HEIGHT_MIDDLE_RIGHT: return "MiddleRight";
  case TARGET_HEIGHT_MIDDLE_LEFT: return "MiddleLeft";
  case TARGET_HEIGHT_LOW: return "Low";
  case TARGET_HEIGHT_MIDDLE_COMBINED: return "MiddleCombined";
  default: return "Unknown";
  }
}

/**
 @brief Determine if the targets are the correct height offset for a computed distance
        i.e. give that we know how far away the target is does it fall into one of the
        three buckets for a low, middle, or high target
*/
void RectTarget::computeTargetType() {
  float lowTargetHeight = RectTargetMeasured::computeLowYOffset(distanceY);
  float middleTargetHeight = RectTargetMeasured::computeMidYOffset(distanceY);
  float highTargetHeight = RectTargetMeasured::computeHighYOffset(distanceY);

  if (RectTargetMeasured::approximateHeight(lowTargetHeight, centerY)) {
    targetType = TARGET_HEIGHT_LOW;
  } else if (RectTargetMeasured::approximateHeight(middleTargetHeight, centerY)) {
    targetType = TARGET_HEIGHT_MIDDLE;
  } else if (RectTargetMeasured::approximateHeight(highTargetHeight, centerY)) {
    targetType = TARGET_HEIGHT_HIGH;
  } else {
    //    targetType = TARGET_HEIGHT_UNKNOWN;
    //HACK
    targetType = TARGET_HEIGHT_LOW;
  }
}

/**
   @brief      Group targets into high, middle and low targets
   @param[in]  image       Image to get size info from
   @param[in]  targets     List of targets to determine which group they fall in
   @param[out] targetGroup Targets grouped into high, middle, and low bins
*/
void RectTarget::computeTargetGroups(Mat &image, vector<RectTarget> &targets,
				     TargetGroup &targetGroup) {
  vector<int> highTargetIndices;
  getTargetsType(targets, highTargetIndices, TARGET_HEIGHT_HIGH);
  if (highTargetIndices.size()) {
    targetGroup.high = targets[highTargetIndices[0]];
  }

  vector<int> lowTargetIndices;
  getTargetsType(targets, lowTargetIndices, TARGET_HEIGHT_LOW);
  if (lowTargetIndices.size()) {
    targetGroup.low = targets[lowTargetIndices[0]];
  }
  
  vector<int> middleTargetIndices;
  getTargetsType(targets, middleTargetIndices, TARGET_HEIGHT_MIDDLE);
  switch (middleTargetIndices.size()) {
    // We have two middle targets
  case 2:
    if (targets[middleTargetIndices[0]].centerX < targets[middleTargetIndices[1]].centerX) {
      targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
      targetGroup.middleLeft = targets[middleTargetIndices[0]];
      targets[middleTargetIndices[1]].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
      targetGroup.middleRight = targets[middleTargetIndices[1]];
    } else {
      targets[middleTargetIndices[1]].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
      targetGroup.middleLeft = targets[middleTargetIndices[1]];
      targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
      targetGroup.middleRight = targets[middleTargetIndices[0]];
    }
    break;
    // We only have one middle target
  case 1:
    // We also have a valid high or low
    if (targetGroup.high.valid || targetGroup.low.valid) {
      float centerX;
      // Get the center target x location
      if (targetGroup.high.valid) {
	centerX = targetGroup.high.centerX;
      } else {
	centerX = targetGroup.low.centerX;
      }

      // Based on the center target location, determine if the middle targets
      // are left or right targets
      if (targets[middleTargetIndices[0]].centerX < centerX) {
	targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
	targetGroup.middleLeft = targets[middleTargetIndices[0]];
      } else {
	targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
	targetGroup.middleRight = targets[middleTargetIndices[0]];
      }

      // If we don't have any center targets, assume that they are they are out of the
      // image.  (i.e. don't assume that they are hidden)
    } else {
      if (targets[middleTargetIndices[0]].centerX > image.cols/2.0) {
	targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
	targetGroup.middleLeft = targets[middleTargetIndices[0]];
      } else {
	targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
	targetGroup.middleRight = targets[middleTargetIndices[0]];
      }
    }
    break;
  }

  // Now determine the location of the center target, given other target data
  if (targetGroup.high.valid) {
    targetGroup.selected = targetGroup.high;
  } else if (targetGroup.low.valid) {
    targetGroup.selected = targetGroup.low;
  } else if (targetGroup.middleLeft.valid && targetGroup.middleRight.valid) {
    targetGroup.selected.centerX = (targetGroup.middleLeft.centerX + targetGroup.middleRight.centerX)/2;
    targetGroup.selected.centerY = (targetGroup.middleLeft.centerY + targetGroup.middleRight.centerY)/2;
    targetGroup.selected.sizeX = (targetGroup.middleLeft.sizeX + targetGroup.middleRight.sizeX)/2;
    targetGroup.selected.sizeY = (targetGroup.middleLeft.sizeY + targetGroup.middleRight.sizeY)/2;
    targetGroup.selected.distanceX = (targetGroup.middleLeft.distanceX + targetGroup.middleRight.distanceX)/2;
    targetGroup.selected.distanceY = (targetGroup.middleLeft.distanceY + targetGroup.middleRight.distanceY)/2;
    targetGroup.selected.angleX = (targetGroup.middleLeft.angleX + targetGroup.middleRight.angleX)/2;
    targetGroup.selected.targetType = TARGET_HEIGHT_MIDDLE_COMBINED;
    targetGroup.selected.valid = true;
  } else if (targetGroup.middleLeft.valid) {
    targetGroup.selected = targetGroup.middleLeft;
  } else if (targetGroup.middleRight.valid) {
    targetGroup.selected = targetGroup.middleRight;
  }
}

/**
   @brief This function is not used.
*/
bool RectTarget::getBestTarget(vector<RectTarget> &targets, RectTarget &target) {
  for (int i=0; i < targets.size(); i++) {
    if (targets[i].targetType == TARGET_HEIGHT_HIGH) {
      target = targets[i];
      return true;
    }
  }
  for (int i=0; i < targets.size(); i++) {
    if (targets[i].targetType == TARGET_HEIGHT_LOW) {
      target = targets[i];
      return true;
    }
  }
  vector <RectTarget>middleTargets;
  for (int i=0; i < targets.size(); i++) {
    if (targets[i].targetType == TARGET_HEIGHT_MIDDLE) {
      middleTargets.push_back(targets[i]);
    }
  }
  switch (middleTargets.size()) {
  case 2:
    if (targets[0].centerX < targets[1].centerX) {
      targets[0].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
      targets[1].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
    } else {
      targets[1].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
      targets[0].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
    }
    target.centerX = (targets[0].centerX + targets[1].centerX)/2;
    target.centerY = (targets[0].centerY + targets[1].centerY)/2;
    target.sizeX = (targets[0].sizeX + targets[1].sizeX)/2;
    target.sizeY = (targets[0].sizeY + targets[1].sizeY)/2;
    target.distanceX = (targets[0].distanceX + targets[1].distanceX)/2;
    target.distanceY = (targets[0].distanceY + targets[1].distanceY)/2;
    target.angleX = (targets[0].angleX + targets[1].angleX)/2;
    target.targetType = TARGET_HEIGHT_MIDDLE_COMBINED;
    return true;
  case 1:
    target = targets[0];
    return true;
  }
    
  return false;
}

/**
   @brief      Filter the list of targets to just those of type targetType
   @param[in]  targets        The list of targets to filter.
   @param[out] targetIndices  The list of targets of type targetType
   @param[in]  targetType     The target type to filter on
*/
void RectTarget::getTargetsType(vector<RectTarget> &targets,
                                vector<int> &targetIndices,
				RectTarget::TargetType targetType) {
  for (int i=0; i < targets.size(); i++) {
    if (targets[i].targetType == targetType) {
      targetIndices.push_back(i);
    }
  }
}



/**
   @brief For each of target compute size, distance, angle
   @param[in]  image        Src image used for sizing
   @param[in]  targetQuads  List of Quads representing targets
   @param[out] targets      Output list of RectTargets computed from targets
*/
void RectTarget::getRectTarget(Mat &image,
			       const vector<vector<Point2f> > &targetQuads,
			       vector<RectTarget> &targets) {
  for (int i=0; i < targetQuads.size(); i++) {
    RectTarget target;
    target.points = targetQuads[i];
    target.valid = true;

    // Compute target center by adding all of the points together and
    // dividing by the number of points
    float centerX = 0;
    float centerY = 0;
    int num_points = target.points.size();
    for (int j=0; j < num_points; j++) {
      float x = target.points[j].x;
      float y = target.points[j].y;
      centerX += x;
      centerY += y;
    }
    target.centerX = centerX/num_points;
    target.centerY = centerY/num_points;

    // Determine which points form lines that are horizontal.  These points
    // will have the greatest x extent
    float x1 = (abs(target.points[0].x - target.points[1].x) +
	      abs(target.points[2].x - target.points[3].x))/2.0;
    float x2 = (abs(target.points[1].x - target.points[2].x) +
	      abs(target.points[3].x - target.points[0].x))/2.0;
    float sizeX = max(x1, x2);

    // Determine which points form lines that are vertical.  These points will
    // have the greatest y extent
    float y1 = (abs(target.points[0].y - target.points[1].y) +
	      abs(target.points[2].y - target.points[3].y))/2.0;
    float y2 = (abs(target.points[1].y - target.points[2].y) +
	      abs(target.points[3].y - target.points[0].y))/2.0;
    float sizeY = max(y1, y2);

    target.sizeX = sizeX;
    target.sizeY = sizeY;
    target.aspectRatio = sizeX/sizeY;
    // Distance to target was obtained from distance to target measurements
    // that were trend line fit in Excel
    target.distanceX = RectTargetMeasured::rectSizeXToDistance(sizeX);
    target.distanceY = RectTargetMeasured::rectSizeYToDistance(sizeY);
    // Angle per pixel is based on the camera perameters
    target.angleX = RectTargetMeasured::pixelsToAngleX((image.cols / 2) - target.centerX);
    target.computeTargetType();

    targets.push_back(target);
  }
}

/**
   @brief Print out a target to the console.
*/
void RectTarget::printTarget() {
  printf("Poly Points[");
  for (int j=0; j < points.size(); j++) {
    printf("(%f, %f) ", points[j].x, points[j].y);
  }
  printf("] ");
  printf("Center (%f, %f) ", centerX, centerY);
  printf("Size (%f, %f) \n", sizeX, sizeY);
}

/**
   @brief Print out a list of targets to the console.
*/
void RectTarget::printTargets(vector<RectTarget> &targets) {
  printf("----------------------------------------------------------------\n");
  for (int i=0; i < targets.size(); i++) {
    targets[i].printTarget();
  }
}

