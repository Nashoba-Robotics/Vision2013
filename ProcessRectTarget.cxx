#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include "Defines.h"
#include "RectTargetMeasured.h"
#include "RectTarget.h"
#include "Messaging.h"
#include "ImageUtils.h"
#include "ProcessRectTarget.h"
#include "VisionDefines.h"
using namespace cv;
using namespace std;

class ProcessingParameters {
public:
  class PlaneWeight {
  public:
    double redWeight;
    double greenWeight;
    double blueWeight;
  };

  class Dilation {
  public:
    int type;
    int size;
  };

  class Threshold {
  public:
    int level;
    //    int adaptiveBlockSize;
  };

  PlaneWeight planeWeight;
  Dilation dilation;
  Threshold threshold;
  double minArea;
  double polygonApproximationEpsilon;
};

// We don't need to process window callbacks because
// we will get the updates in the main loop
static void dummyCallback(int, void* ) {}

void ProcessRectTarget::targetsDebugText(Mat &finalDrawing, RectTarget &target) {
#ifdef DEBUG_TEXT
  Point textAlign( target.centerX - 50, target.centerY + 35 );
  Point sizeAlign( target.centerX - 50, target.centerY + 50 );
  Point distanceAlign( target.centerX - 50, target.centerY + 65 );
  Point angleXAlign( target.centerX - 50, target.centerY + 80 );
  Point typeTargetAlign( target.centerX - 50, target.centerY + 95 );
  ostringstream text, size, distance, angle, typeTarget;
  text << "Center: X: " << target.centerX << " Y: " << target.centerY;
  distance << "Distance: X: " << target.distanceX << " Y: " << target.distanceY;
  size << "Size: X: " << target.sizeX << " Y: " << target.sizeY;
  angle << "Angle: X: " << target.angleX;
  typeTarget << target.getTargetTypeString();
  putText(finalDrawing, text.str(), textAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
  putText(finalDrawing, size.str(), sizeAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
  putText(finalDrawing, distance.str(), distanceAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
  putText(finalDrawing, angle.str(), angleXAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
  putText(finalDrawing, typeTarget.str(), typeTargetAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
#endif
}

// This is called every time that we get a new image to process the image, get target data,
// and send the information to the crio
void ProcessRectTarget::processImage(Mat &srcImage, Mat &displayImage, Mat &finalImage, bool guiAll,
				     EventRate &eventRate) {
  //  static vector<Mat> planes;
  //  static Mat grayScale;
  GaussianBlur(srcImage, images.blurred, Size( 5, 5 ), 0, 0 );
  split(images.blurred, images.planes);
  
  // Keep the color that we are intested in and substract off the other planes
  // GREEN - .5 * RED - .5 * BLUE
  double redWeight   = 2*(params.trackRed - params.trackOffset) / params.trackMax;
  double greenWeight = 2*(params.trackGreen - params.trackOffset) / params.trackMax;
  double blueWeight  = 2*(params.trackBlue - params.trackOffset) / params.trackMax;
  //  printf("%f %f %f\n", redWeight, greenWeight, blueWeight);
  addWeighted(images.planes[GREEN_PLANE], greenWeight,
	      images.planes[RED_PLANE], redWeight, 0, images.grayScale);
  addWeighted(images.grayScale, 1,
	      images.planes[BLUE_PLANE], blueWeight, 0, images.grayScale);
  //grayScale = planes[GREEN_PLANE];
  
  //static Mat blurred;
  //  images.blurred = images.grayScale.clone();
  //  GaussianBlur(images.grayScale, images.blurred, Size( 5, 5 ), 0, 0 );
  
  //static Mat thresholded;
  //  images.thresholded = images.blurred.clone();
  
  cvtColor(images.blurred, images.hsvImage, CV_RGB2HSV);
  split(images.hsvImage, images.planes);

  HsvThreshold::Params hsvParams = hsvThreshold.getParams();
  inRange(images.hsvImage,  
	  Scalar( hsvParams.huelow, hsvParams.satlow, hsvParams.valuelow),  
	  Scalar( hsvParams.huehigh, hsvParams.sathigh, hsvParams.valuehigh),  
	  images.thresholded);
  //  inRange(images.hsvImage,  
  //	  Scalar(params.huelow, params.satlow, params.valuelow),  
  //	  Scalar(params.huehigh, params.sathigh, params.valuehigh),  
  //	  images.thresholded);  

  /// Detect edges using Threshold
  //  threshold(images.blurred, images.thresholded,
  //  	    thresh, 255, THRESH_BINARY );
  //  thresh_block_size = (thresh_block_size/2)*2+1;
  //  thresh_block_size = max(thresh_block_size,3);
  //  adaptiveThreshold( images.blurred, images.thresholded,
  //		     255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, thresh_block_size, 0);

  // Dilation + Erosion = Close
  int dilationType;
  if( params.dilation_elem == 0 ) { dilationType = MORPH_RECT; }
  else if( params.dilation_elem == 1 ) { dilationType = MORPH_CROSS; }
  else if( params.dilation_elem == 2) { dilationType = MORPH_ELLIPSE; }
  
  Mat element = getStructuringElement(dilationType,
				      Size( 2*params.dilation_size + 1, 2*params.dilation_size+1 ),
				      Point( params.dilation_size, params.dilation_size ) );
  // This now does a close
  //  static Mat dilated;
  dilate(images.thresholded, images.dilated, element );
  erode(images.dilated, images.dilated, element);

  vector<vector<Point> > contours;
  static Mat contoursMat;
  contoursMat = images.dilated.clone();
  /// Find contours
  //  findContours( contoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  vector<Vec4i> hierarchy;
  findContours( contoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0) );
  
  /// Find the convex hull object for each contour
  vector<vector<Point> > hull( contours.size() );
  for( int i = 0; i < contours.size(); i++ ) {
    convexHull( Mat(contours[i]), hull[i], false );
  }
  
  // Approximate the convect hulls with pologons
  // This reduces the number of edges and makes the contours
  // into quads
  vector<vector<Point> > poly( contours.size() );
  for (int i=0; i < contours.size(); i++) {
    approxPolyDP(hull[i], poly[i], params.poly_epsilon, true);
    // These come out reversed, so reverse back
    reverse(poly[i].begin(), poly[i].end());
  }
  
  // Prune the polygons into only the ones that we are intestered in.
  vector<vector<Point> > prunedPoly(0);
  vector<vector<Point> > prunedHulls(0);
  vector<vector<Point> > prunedContours(0);
  for (int i=0; i < poly.size(); i++) {
    // Only 4 sized figures
    if (poly[i].size() == 4) {
      Rect bRect = boundingRect(poly[i]);
      // Remove polygons that are too small
      if (bRect.width * bRect.height > params.minArea) {
	prunedPoly.push_back(poly[i]);
	prunedHulls.push_back(hull[i]);
	prunedContours.push_back(contours[i]);
      }
    }
  }
  
  // Prune to targets (Rectangles that contain an inner rectangle
  vector<vector<Point> >targetQuads(0);
  vector<vector<Point> >targetHulls(0);
  vector<vector<Point> >targetContours(0);
  for (int i=0; i < prunedPoly.size(); i++) {
    // Keep only polygons that contain other polygons
    if (ImageUtils::rectContainsRect(i, prunedPoly)) {
      targetQuads.push_back(prunedPoly[i]);
      targetHulls.push_back(prunedHulls[i]);
      targetContours.push_back(prunedContours[i]);
    }
  }

  //Refine corner locations
  vector<vector<Point2f> >targetQuads2f(targetQuads.size());
  vector<vector<Point> >targetQuads2fi(targetQuads.size());
  vector<vector<Point> > rcontours = targetContours;
  ImageUtils::refineCorners(targetQuads, rcontours, targetQuads2f, targetQuads2fi);
  
  vector<RectTarget> targets;
  RectTarget::getRectTarget(srcImage, targetQuads2f, targets);

  vector<RectTarget> prunedTargets(0);
  vector<vector<Point> >prunedTargetQuads2fi(0);
  vector<vector<Point> >prunedTargetQuads(0);
  float aspectRatio = 3.1;
  for (int i=0; i < targets.size(); i++) {
    if ((targets[i].aspectRatio < (aspectRatio *1.1)) &&
	(targets[i].aspectRatio > (aspectRatio * .9))) {
      prunedTargets.push_back(targets[i]);
      prunedTargetQuads2fi.push_back(targetQuads2fi[i]);
      prunedTargetQuads.push_back(targetQuads[i]);
    }
  }
  
  TargetGroup targetGroup;
  RectTarget::computeTargetGroups(srcImage, prunedTargets, targetGroup);

  //  printTargets(targets);
  
  // Output the final image
  finalImage = srcImage.clone();
  // Show first approx for lines
  for (int i=0; i < prunedTargetQuads.size(); i++) {
    drawContours(finalImage, prunedTargetQuads, i, ColorBlue, 1, 8, vector<Vec4i>(), 0, Point() );
  }
  for (int i=0; i < prunedTargetQuads2fi.size(); i++) {
    drawContours(finalImage, prunedTargetQuads2fi, i, ColorWhite, 1, 8, vector<Vec4i>(), 0, Point() );
  }

#ifdef DEBUG_TEXT
  Point fpsAlign( 10, 10 );
  ostringstream fpsText;
  fpsText.precision(3);
  fpsText << "Rect target Avg FPS=" << eventRate.avgFps << " Filt FPS=" << eventRate.filterFps << " FPS="<< eventRate.fps ;
  putText( finalImage, fpsText.str(), fpsAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite );
#endif

  for (int i = 0; i < prunedTargets.size(); i++ ) {
    Point center( prunedTargets[i].centerX, prunedTargets[i].centerY );
    circle( finalImage, center, 10, ColorWhite );
    targetsDebugText(finalImage, prunedTargets[i]);
  }
  Point topCenterImagePt(finalImage.cols/2, 0);
  Point bottomCenterImagePt(finalImage.cols/2, finalImage.rows);
  line(finalImage, topCenterImagePt, bottomCenterImagePt, ColorWhite, 2);

  // Display a horizontal line so that we know that we are at the correct distance
  int targetLineHeight = .3 * finalImage.rows;
  Point rightTargetImagePt(0, targetLineHeight);
  Point leftTargetImagePt(finalImage.cols, targetLineHeight);
  line(finalImage, rightTargetImagePt, leftTargetImagePt, ColorWhite, 2);

  // If we have a target then send it to the cRio
  if (targetGroup.selected.valid) {
    Point center( targetGroup.selected.centerX, targetGroup.selected.centerY );
    double thickness = (abs(targetGroup.selected.angleX) < 2) ? 5 : 1;
    circle( finalImage, center, 15, ColorPurple, thickness);
    //    printf("dist=%f angle=%f type=%s\n", targetGroup.selected.distanceY,
    //	   targetGroup.selected.angleX,
    //	   targetGroup.selected.getTargetTypeString());
    //#ifdef CRIO_NETWORK
    float tension = RectTargetMeasured::convertDistanceToTension(targetGroup.selected.distanceY);
    //    printf("Here\n");
    sendMessageRect(CRIO_IP_ADDR, targetGroup.selected.distanceY, targetGroup.selected.angleX, tension);
    //#endif
  }

  switch (imageId) {
  case imageIdGrayScale: displayImage = ImageUtils::convertToRGB(images.grayScale.clone()); break;
  case imageIdBlurred:   displayImage = ImageUtils::convertToRGB(images.blurred.clone()); break;
  case imageIdDilated:   displayImage = ImageUtils::convertToRGB(images.dilated.clone()); break;
  case imageIdHSV:            displayImage = ImageUtils::convertToRGB(images.hsvImage.clone()); break;
  case imageIdThresholded:    displayImage = ImageUtils::convertToRGB(images.thresholded.clone()); break;
  case imageIdHistogram: {
    ImageUtils::calcHistogram(srcImage, images.histImage);
    displayImage = ImageUtils::convertToRGB(images.histImage.clone()); break;
  }
  case imageIdContours: {
    images.contours = Mat::zeros(srcImage.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ ) {
      drawContours(images.contours, contours, i, ColorWhite, 1, 8,
		   vector<Vec4i>(), 0, Point() );
    }
    displayImage = ImageUtils::convertToRGB(images.contours.clone());
    break;
  }
  case imageIdPolygons:  {
    // Draw the contours in a window
    images.polygons = Mat::zeros(images.thresholded.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ ) {
      drawContours(images.polygons, poly, i, ColorWhite, 1, 8,
		   vector<Vec4i>(), 0, Point() );
    }

    displayImage = ImageUtils::convertToRGB(images.polygons.clone());
    break;
  }
  case imageIdPrunedPolygons: {
    // Draw the pruned Poloygons in a window
    images.prunedPolygons = Mat::zeros(images.thresholded.size(), CV_8UC3 );
    for (int i=0; i < prunedPoly.size(); i++) {
      drawContours(images.prunedPolygons, prunedPoly, i, ColorWhite, 1, 8,
		   vector<Vec4i>(), 0, Point() );
    }
    
    displayImage = ImageUtils::convertToRGB(images.prunedPolygons.clone());
    break;
  }
  case imageIdTargets: {
    // Draw the targets
    //Mat
    images.targets = Mat::zeros( images.thresholded.size(), CV_8UC3 );
    for (int i=0; i < targetQuads.size(); i++) {
      drawContours(images.targets, targetQuads, i, ColorGray, 1, 8, vector<Vec4i>(), 0, Point() );
    }
    
    // Draw the targets
    //Mat targets2fi = Mat::zeros( thresholded.size(), CV_8UC3 );
    for (int i=0; i < targetQuads2fi.size(); i++) {
      drawContours(images.targets, targetQuads2fi, i, ColorWhite, 1, 8, vector<Vec4i>(), 0, Point() );
    }
    
    displayImage = ImageUtils::convertToRGB(images.targets.clone());
    break;
  }
  case imageIdPrunedTargets: {
    // Draw the targets
    //Mat
    images.prunedTargets = Mat::zeros( images.thresholded.size(), CV_8UC3 );
    for (int i=0; i < prunedTargetQuads.size(); i++) {
      drawContours(images.prunedTargets, prunedTargetQuads, i, ColorGray, 1, 8, vector<Vec4i>(), 0, Point() );
    }
    
    // Draw the targets
    //Mat targets2fi = Mat::zeros( thresholded.size(), CV_8UC3 );
    for (int i=0; i < prunedTargetQuads2fi.size(); i++) {
      drawContours(images.prunedTargets, prunedTargetQuads2fi, i, ColorWhite, 1, 8, vector<Vec4i>(), 0, Point() );
    }
    
    displayImage = ImageUtils::convertToRGB(images.prunedTargets.clone());
    break;
  }
  case imageIdFinal:          displayImage = ImageUtils::convertToRGB(finalImage.clone()); break;
  default: printf("Illegal image case\n"); exit(-1);
  }
    //    	sleep(1);
}

ProcessRectTarget::ProcessRectTarget() : imageId(imageIdFinal),
					 hsvThreshold(false,
						      Vision::RectTarget::HSVThreshold::HueLow,
						      Vision::RectTarget::HSVThreshold::HueHigh,
						      0, 0,
						      Vision::RectTarget::HSVThreshold::SatLow,
						      Vision::RectTarget::HSVThreshold::SatHigh,
						      Vision::RectTarget::HSVThreshold::ValueLow,
						      Vision::RectTarget::HSVThreshold::ValueHigh)
{
  params.thresh = 113;                   //!< Defines the threshold level to apply to image
  params.thresh_block_size = 23;        //!< Defines the threshold block size to apply to image
  params.max_thresh_block_size = 255;   //!< Defines the threshold block size to apply to image
  params.max_thresh = 255;              //!< Max threshold for the trackbar
  params.poly_epsilon = 10;             //!< Epsilon to determine reduce number of poly sides
  params.max_poly_epsilon = 50;         //!< Max epsilon for the trackbar
  
  params.trackRed = 10;
  params.trackGreen = 20;
  params.trackBlue = 10;
  params.trackOffset = 10;
  params.trackMax = 20;
  params.max_colorsize = 20;
  
  params.minArea = 500;                 //!< Polygons below this size are rejected
  params.max_minArea = 10000;           //!< Max polygon size for the trackbar
  
  params.dilation_elem = 0;             //!< The element type dilate/erode with
  params.max_elem = 2;            //!< Max trackbar element type
  params.dilation_size = 4;             //!< The size of the element to dilate/erode with
  params.max_kernel_size = 21;    //!< Max trackbar kernel dialation size
  
  params.erode_count = 1;               //!< The number of times to erode the image
  params.erode_max = 20;                //!< Max number of times to erode on trackbar
  
  params.target_width_inches = 24;      //!< Width of a physical target in inches
  params.target_height_inches = 16;     //!< Height of a physical target in inches

  imageNames.resize(12);
  imageNames[imageIdGrayScale] = "Gray Scale Image";
  imageNames[imageIdHistogram] = "Histogram Image";
  imageNames[imageIdBlurred] = "Blurred Image";
  imageNames[imageIdDilated] = "Dilated Image";
  imageNames[imageIdHSV] = "HSV Image";
  imageNames[imageIdThresholded] = "Thresholded Image";
  imageNames[imageIdContours] = "Contours Image";
  imageNames[imageIdPolygons] = "Polygons Image";
  imageNames[imageIdPrunedPolygons] = "Pruned Polygons Image";
  imageNames[imageIdTargets] = "Targets Image";
  imageNames[imageIdPrunedTargets] = "Pruned Targets Image";
  imageNames[imageIdFinal] = "Final Image";
}

void ProcessRectTarget::initGui(bool guiAll) {
  /// Create Window
  if (guiAll) {
    createTrackbar("Red", "GrayScaleImage", &params.trackRed, params.max_colorsize, dummyCallback);
    createTrackbar("Green", "GrayScaleImage", &params.trackGreen, params.max_colorsize, dummyCallback);
    createTrackbar("Blue", "GrayScaleImage", &params.trackBlue, params.max_colorsize, dummyCallback);

    createTrackbar("minArea", "PrunedPolygon", &params.minArea, params.max_minArea, dummyCallback);

    //    createTrackbar("threshold", "Threshold", &params.thresh, params.max_thresh, dummyCallback);
    //    createTrackbar("block size", "Threshold", &params.thresh_block_size, params.max_thresh_block_size, dummyCallback);

    createTrackbar("Poly epsilon", "Polygon", &params.poly_epsilon, params.max_poly_epsilon, dummyCallback);
    createTrackbar("Element: 0:Rect 1:Cross 2:Ellipse", "Dilate", &params.dilation_elem, params.max_elem, dummyCallback);
    createTrackbar("Kernel size:\n 2n+1", "Dilate", &params.dilation_size, params.max_kernel_size, dummyCallback);
    createTrackbar("Erode", "Dilate", &params.erode_count, params.erode_max, dummyCallback);
  }
}

