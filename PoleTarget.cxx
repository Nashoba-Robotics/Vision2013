#include <stdio.h>
#include <math.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "PoleTarget.h"
#include "PoleTargetMeasured.h"

using namespace cv;
using namespace std;

/**
   @brief Default contructor
*/
PoleTarget::PoleTarget() {
  centerX = 0;
  centerY = 0;
  sizeX = 0;
  sizeY = 0;
  distanceX = 0;
  distanceY = 0;
  angleX = 0;
  tension = 0;
  valid = 0;
}

Point2f averagePoints(Point2f p1, Point2f p2) {
  Point2f ret;
  ret.x = (p1.x + p2.x)/2;
  ret.y = (p1.y + p2.y)/2;
  return ret;
}

/**
   @brief For each of target compute size, distance, angle
   @param[in]  image        Src image used for sizing
   @param[in]  targetQuads  List of Quads representing targets
   @param[out] targets      Output list of PoleTargets computed from targets
*/
void PoleTarget::getPoleTargets(Mat &image,
			       const vector<vector<Point2f> > &targetQuads,
			       vector<PoleTarget> &targets) {
  for (int i=0; i < targetQuads.size(); i++) {
    PoleTarget target;
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

    if (y1 > y2) {
      if (target.points[0].y > target.points[1].y) {
	target.topPt = averagePoints(target.points[0],target.points[3]);
	target.bottomPt = averagePoints(target.points[1], target.points[2]);
      } else {
	target.topPt = averagePoints(target.points[1], target.points[2]);
	target.bottomPt = averagePoints(target.points[0], target.points[3]);
      }
    } else {
      if (target.points[1].y > target.points[2].y) {
	target.topPt = averagePoints(target.points[1], target.points[0]);
	target.bottomPt = averagePoints(target.points[2], target.points[3]);
      } else {
	target.topPt = averagePoints(target.points[2], target.points[3]);
	target.bottomPt = averagePoints(target.points[1], target.points[0]);
      }
    }
    Point2f angleLine = target.topPt - target.bottomPt;
    #define PI 3.1415926535
    double anglePoleRadians = atan(angleLine.y/angleLine.x);
    target.anglePole = anglePoleRadians * 180.0 / PI;

    double z = 30;     // 30 inches
    double r = 17.32;  // 17.32 inches 
    target.anglePerpendicular = asin(z/(r * tan(anglePoleRadians))) * 180.0 / PI;
    float sizeY = max(y1, y2);

    target.sizeX = sizeX;
    target.sizeY = sizeY;
    // Distance to target was obtained from distance to target measurements
    // that were trend line fit in Excel
    target.distanceX = PoleTargetMeasured::rectSizeXToDistance(sizeX);
    target.distanceY = PoleTargetMeasured::rectSizeYToDistance(sizeY);
    // Angle per pixel is based on the camera perameters
    target.angleX = PoleTargetMeasured::pixelsToAngleX((image.cols / 2) - target.centerX);

    targets.push_back(target);
  }
}

PoleTarget& PoleTarget::getBestPoleTarget(std::vector<PoleTarget> &targets, PoleTarget &bestTarget) {
  if (!targets.size()) {
    bestTarget.valid = false;
    return bestTarget;
  }

  bestTarget = targets[0];
  for (int i = 1; i < targets.size(); i++) {
    if ((targets[i].sizeX * targets[i].sizeY) > (bestTarget.sizeX * bestTarget.sizeY)) {
      bestTarget = targets[i];
    }
  }
  return bestTarget;
}

/**
   @brief Print out a target to the console.
*/
void PoleTarget::printTarget() {
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
void PoleTarget::printTargets(vector<PoleTarget> &targets) {
  printf("----------------------------------------------------------------\n");
  for (int i=0; i < targets.size(); i++) {
    targets[i].printTarget();
  }
}

