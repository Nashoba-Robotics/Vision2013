#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "Arguments.h"
#include "ImageUtils.h"
#include "RectTargetMeasured.h"
#include "RectTarget.h"
#include "Messaging.h"
#include "TimeUtils.h"

using namespace cv;
using namespace std;

#define DEBUG_TEXT            1
//#define DEBUG_PROCESSING_TIME 1
#define CRIO_NETWORK      1           // Normal case
//#define WPI_IMAGES               // For debugging with WPI images

// Image Plane definitions
int BLUE_PLANE  = 0;
int GREEN_PLANE = 1;
int RED_PLANE   = 2;

static const Scalar ColorWhite  = Scalar( 255, 255, 255 );
static const Scalar ColorPurple = Scalar( 255, 0, 255 );
static const Scalar ColorGray   = Scalar( 64, 64, 64 );
static const Scalar ColorBlue   = Scalar( 255, 0, 0 );

#define CRIO_IP_ADDR "10.17.68.2"
#define CAMERA_HTTP_ADDR "http://10.17.68.9/axis-cgi/mjpg/video.cgi?resolution=320x240&req_fps=30&.mjpg"
//   CAMERA_HTTP_ADDR "http://10.17.68.9/axis-cgi/mjpg/video.cgi?resolution=320x240&req_fps=60&.mjpg"

bool pause_image = false;
int thresh = 113;                   //!< Defines the threshold level to apply to image
int thresh_block_size = 23;        //!< Defines the threshold block size to apply to image
int max_thresh_block_size = 255;   //!< Defines the threshold block size to apply to image
int max_thresh = 255;              //!< Max threshold for the trackbar
int poly_epsilon = 10;             //!< Epsilon to determine reduce number of poly sides
int max_poly_epsilon = 50;         //!< Max epsilon for the trackbar

int trackRed = 10;
int trackGreen = 20;
int trackBlue = 10;
double trackOffset = 10;
double trackMax = 20;
int max_colorsize = 20;

int minArea = 500;                 //!< Polygons below this size are rejected
int max_minArea = 10000;           //!< Max polygon size for the trackbar

int dilation_elem = 0;             //!< The element type dilate/erode with
int const max_elem = 2;            //!< Max trackbar element type
int dilation_size = 4;             //!< The size of the element to dilate/erode with
int const max_kernel_size = 21;    //!< Max trackbar kernel dialation size

int erode_count = 1;               //!< The number of times to erode the image
int erode_max = 20;                //!< Max number of times to erode on trackbar

int target_width_inches = 24;      //!< Width of a physical target in inches
int target_height_inches = 16;     //!< Height of a physical target in inches

Mat srcImage;
EventRate eventRate;

// Command line argument processing
OptionsProcess options;

// We don't need to process window callbacks because
// we will get the updates in the main loop
void dummyCallback(int, void* ) {}


void createGuiWindows() {
  /// Create Window
  if (options.guiAll) {
    namedWindow("Source", CV_WINDOW_AUTOSIZE );
    namedWindow("Red", CV_WINDOW_AUTOSIZE );
    namedWindow("Blue", CV_WINDOW_AUTOSIZE );
    namedWindow("Green", CV_WINDOW_AUTOSIZE );
    namedWindow("GrayScaleImage", CV_WINDOW_AUTOSIZE );
    namedWindow("Blur", CV_WINDOW_AUTOSIZE );
    namedWindow("Dilate", CV_WINDOW_AUTOSIZE );
    namedWindow("Threshold", CV_WINDOW_AUTOSIZE );
    namedWindow("Contours", CV_WINDOW_AUTOSIZE );
    namedWindow("Polygon", CV_WINDOW_AUTOSIZE );
    namedWindow("PrunedPolygon", CV_WINDOW_AUTOSIZE );
    namedWindow("Targets", CV_WINDOW_AUTOSIZE );
    namedWindow("Final", CV_WINDOW_AUTOSIZE | CV_GUI_EXPANDED);

    createTrackbar("Red", "GrayScaleImage", &trackRed, max_colorsize, dummyCallback);
    createTrackbar("Green", "GrayScaleImage", &trackGreen, max_colorsize, dummyCallback);
    createTrackbar("Blue", "GrayScaleImage", &trackBlue, max_colorsize, dummyCallback);

    createTrackbar("minArea", "PrunedPolygon", &minArea, max_minArea, dummyCallback);

    createTrackbar("threshold", "Threshold", &thresh, max_thresh, dummyCallback);
    createTrackbar("block size", "Threshold", &thresh_block_size, max_thresh_block_size, dummyCallback);

    createTrackbar("Poly epsilon", "Polygon", &poly_epsilon, max_poly_epsilon, dummyCallback);
    createTrackbar("Element: 0:Rect 1:Cross 2:Ellipse", "Dilate", &dilation_elem, max_elem, dummyCallback);
    createTrackbar("Kernel size:\n 2n+1", "Dilate", &dilation_size, max_kernel_size, dummyCallback);
    createTrackbar("Erode", "Dilate", &erode_count, erode_max, dummyCallback);
  }
}

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

class ProcessingData {
public:
  Mat srcImage;
  Mat grayScaleImage;
  Mat blurredImage;
  Mat dilatedImage;
  Mat thresholdedImage;
  Mat contoursImage;
  Mat polygonImage;
  Mat prunedPolygonImage;
  Mat targetsImage;

  vector<Mat> planes;
};

ProcessingData processingData;

// This is called every time that we get a new image to process the image, get target data,
// and send the information to the crio
void processImageCallback(int, void* ) {
  static vector<Mat> planes;
  static Mat grayScaleImage;
  split(srcImage, planes);
  
  // Keep the color that we are intested in and substract off the other planes
  // GREEN - .5 * RED - .5 * BLUE
  double redWeight   = (trackRed - trackOffset) / trackMax;
  double greenWeight = (trackGreen - trackOffset) / trackMax;
  double blueWeight  = (trackBlue - trackOffset) / trackMax;
  //  printf("%f %f %f\n", redWeight, greenWeight, blueWeight);
  addWeighted(planes[GREEN_PLANE], greenWeight, planes[RED_PLANE], redWeight, 0, grayScaleImage);
  addWeighted(grayScaleImage, 1, planes[BLUE_PLANE], blueWeight, 0, grayScaleImage);
  //grayScaleImage = planes[GREEN_PLANE];
  
  static Mat blurredImage;
  blurredImage = grayScaleImage.clone();
  GaussianBlur( grayScaleImage, blurredImage, Size( 5, 5 ), 0, 0 );
  
  static Mat thresholdedImage;
  thresholdedImage = blurredImage.clone();
  vector<Vec4i> hierarchy;
  
  /// Detect edges using Threshold
  threshold( blurredImage, thresholdedImage, thresh, 255, THRESH_BINARY );
  //  thresh_block_size = (thresh_block_size/2)*2+1;
  //  thresh_block_size = max(thresh_block_size,3);
  //  adaptiveThreshold( grayScaleImage, thresholdedImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, thresh_block_size, 0);

  // Dilation + Erosion = Close
  int dilationType;
  if( dilation_elem == 0 ) { dilationType = MORPH_RECT; }
  else if( dilation_elem == 1 ) { dilationType = MORPH_CROSS; }
  else if( dilation_elem == 2) { dilationType = MORPH_ELLIPSE; }
  
  Mat element = getStructuringElement(dilationType,
				      Size( 2*dilation_size + 1, 2*dilation_size+1 ),
				      Point( dilation_size, dilation_size ) );
  // This now does a close
  static Mat dilatedImage;
  dilate(thresholdedImage, dilatedImage, element );
  erode(dilatedImage, dilatedImage, element);

  vector<vector<Point> > contours;
  static Mat contoursMat;
  contoursMat = dilatedImage.clone();
  /// Find contours
  //  findContours( contoursMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
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
    approxPolyDP(hull[i], poly[i], poly_epsilon, true);
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
      if (bRect.width * bRect.height > minArea) {
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
  TargetGroup targetGroup;
  RectTarget::computeTargetGroups(srcImage, targets, targetGroup);

  //  printTargets(targets);
  
  if (options.guiAll) {
    static Mat contoursImage;
    /// Draw contours + hull results
    contoursImage = Mat::zeros( thresholdedImage.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ ) {
      drawContours( contoursImage, contours, i, ColorWhite, 1, 8, vector<Vec4i>(), 0, Point() );
    }

    // Draw the contours in a window
    Mat polygonImage = Mat::zeros( thresholdedImage.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ ) {
      drawContours( polygonImage, poly, i, ColorWhite, 1, 8, vector<Vec4i>(), 0, Point() );
    }
    
    // Draw the pruned Poloygons in a window
    Mat prunedPolygonImage = Mat::zeros( thresholdedImage.size(), CV_8UC3 );
    for (int i=0; i < prunedPoly.size(); i++) {
      drawContours(prunedPolygonImage, prunedPoly, i, ColorWhite, 1, 8, vector<Vec4i>(), 0, Point() );
    }
    
    // Draw the targets
    Mat targetsImage = Mat::zeros( thresholdedImage.size(), CV_8UC3 );
    for (int i=0; i < targetQuads.size(); i++) {
      drawContours(targetsImage, targetQuads, i, ColorGray, 1, 8, vector<Vec4i>(), 0, Point() );
    }
    
    // Draw the targets
    //Mat targetsImage2fi = Mat::zeros( thresholdedImage.size(), CV_8UC3 );
    for (int i=0; i < targetQuads2fi.size(); i++) {
      drawContours(targetsImage, targetQuads2fi, i, ColorWhite, 1, 8, vector<Vec4i>(), 0, Point() );
    }
    ImageUtils::calcHistogram(srcImage);
    imshow("Source", srcImage);
    imshow("Red", planes[RED_PLANE]);
    imshow("Blue", planes[BLUE_PLANE]);
    imshow("Green", planes[GREEN_PLANE]);
    imshow("GrayScaleImage", grayScaleImage);
    imshow("Blur", blurredImage);
    imshow("Dilate", dilatedImage);
    imshow("Threshold", thresholdedImage);
    imshow("Contours", contoursImage);
    imshow("Polygon", polygonImage );
    imshow("PrunedPolygon", prunedPolygonImage);
    imshow("Targets", targetsImage);
  }

  // Output the final image
  static  Mat finalDrawing;
  finalDrawing = srcImage.clone();
  // Show first approx for lines
    for (int i=0; i < targetQuads.size(); i++) {
      drawContours(finalDrawing, targetQuads, i, ColorBlue, 1, 8, vector<Vec4i>(), 0, Point() );
    }
  for (int i=0; i < targetQuads2fi.size(); i++) {
    drawContours(finalDrawing, targetQuads2fi, i, ColorWhite, 1, 8, vector<Vec4i>(), 0, Point() );
  }

#ifdef DEBUG_TEXT
  Point fpsAlign( 10, 10 );
  ostringstream fpsText;
  fpsText.precision(3);
  fpsText << "Avg FPS=" << eventRate.avgFps << " Filt FPS=" << eventRate.filterFps << " FPS="<< eventRate.fps ;
  putText( finalDrawing, fpsText.str(), fpsAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite );
#endif

  for (int i = 0; i < targets.size(); i++ ) {
    Point center( targets[i].centerX, targets[i].centerY );
    circle( finalDrawing, center, 10, ColorWhite );
#ifdef DEBUG_TEXT
    Point textAlign( targets[i].centerX - 50, targets[i].centerY + 35 );
    Point sizeAlign( targets[i].centerX - 50, targets[i].centerY + 50 );
    Point distanceAlign( targets[i].centerX - 50, targets[i].centerY + 65 );
    Point angleXAlign( targets[i].centerX - 50, targets[i].centerY + 80 );
    Point typeTargetAlign( targets[i].centerX - 50, targets[i].centerY + 95 );
    Point tensionAlign( targets[i].centerX - 50, targets[i].centerY + 110 );
    ostringstream text, size, distance, angle, typeTarget, tension;
    text << "Center: X: " << targets[i].centerX << " Y: " << targets[i].centerY;
    distance << "Distance: X: " << targets[i].distanceX << " Y: " << targets[i].distanceY;
    size << "Size: X: " << targets[i].sizeX << " Y: " << targets[i].sizeY;
    angle << "Angle: X: " << targets[i].angleX;
    typeTarget << targets[i].getTargetTypeString();
    tension << "Tension: " << targets[i].tension;
    putText( finalDrawing, text.str(), textAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
    putText( finalDrawing, size.str(), sizeAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
    putText( finalDrawing, distance.str(), distanceAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
    putText( finalDrawing, angle.str(), angleXAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
    putText( finalDrawing, typeTarget.str(), typeTargetAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
    putText( finalDrawing, tension.str(), tensionAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite);
#endif  
  }
  // If we have a target then send it to the cRio
  if (targetGroup.selected.valid) {
    Point center( targetGroup.selected.centerX, targetGroup.selected.centerY );
    double thickness = (abs(targetGroup.selected.angleX) < 2) ? 5 : 1;
    circle( finalDrawing, center, 15, ColorPurple, thickness);
    //    printf("dist=%f angle=%f type=%s\n", targetGroup.selected.distanceY,
    //	   targetGroup.selected.angleX,
    //	   targetGroup.selected.getTargetTypeString());
    //#ifdef CRIO_NETWORK
    float tension = RectTargetMeasured::convertDistanceToTension(targetGroup.selected.distanceY);
    //    printf("Here\n");
    sendMessage(CRIO_IP_ADDR, targetGroup.selected.distanceY, targetGroup.selected.angleX, tension);
    //#endif
  }
  
  imshow( "Final", finalDrawing );
}

int main( int argc, char** argv ) {
  VideoCapture *cap = 0;
  VideoWriter *record = 0;
  
  // Process any opencv arguments
  cvInitSystem(argc, argv);
  options.processArgs(argc, argv);

  if (options.processCamera) {
    if (options.processVideoFile) {
      cap = new VideoCapture(options.fileName);
    } else {
      cap = new VideoCapture(CAMERA_HTTP_ADDR); 
      //cap = new VideoCapture(0); // open the default camera
      //      cap->set(CV_CAP_PROP_FPS, 60);
      //      cap->set(CV_CAP_PROP_FRAME_WIDTH, 640);
      //      cap->set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    }
    if(!cap || !cap->isOpened()) { // check if we succeeded
      printf("ERROR: unable to open camera\n");
      return -1;
    }
    *cap >> srcImage;
    string outputFileName = ImageUtils::getOutputVideoFileName();
    record = new VideoWriter(outputFileName.c_str(), CV_FOURCC('M', 'J', 'P', 'G'), 30, srcImage.size(), true);
    if (!record->isOpened()) {
      printf("VideoWriter failed to open!\n");
      return -1;
    }
  } else {
    // Load an image from a file
    srcImage = imread(options.fileName, 1 );
  }

  createGuiWindows();

  eventRate.init();
  
  //  dilation_callback(0,0);
  while (1) {
    timespec time1, time2, time3, time4, time5;

    clock_gettime(CLOCK_REALTIME, &time1);
    if (options.processCamera) {
      //      cap->set(CV_CAP_PROP_POS_AVI_RATIO, 1);
      if (!pause_image) {
	// If there are more images to process then process them else
	// return.  This will be the case when processing an image file.
	if (!cap->grab())
	  return 0;
	if (!cap->retrieve(srcImage))
	  return 0;
	//cap >> srcImage;
	*record << srcImage;
      }
    }
    clock_gettime(CLOCK_REALTIME, &time2);

    processImageCallback( 0, 0 );
    clock_gettime(CLOCK_REALTIME, &time3);

    char c = 0;

    if (options.processCamera) {
      static int counter = 0;
      counter++;
      if ((counter % 5) == 0)
        c = waitKey(1);
    } else {
      // Non realtime images
      c = waitKey(1);
    }
    clock_gettime(CLOCK_REALTIME, &time4);

    if (c == 'q') return 0;
    if (c == 'p') pause_image = !pause_image;
    if (c == 'w') ImageUtils::writeImage(srcImage);
    clock_gettime(CLOCK_REALTIME, &time5);
    #ifdef DEBUG_PROCESSING_TIME
    printf("Retrieve:         %0ld:%09ld\n", TimeUtils::diff(time1,time2).tv_sec, TimeUtils::diff(time1,time2).tv_nsec);
    printf("Process callback: %0ld:%09ld\n", TimeUtils::diff(time2,time3).tv_sec, TimeUtils::diff(time2,time3).tv_nsec);
    printf("Wait:             %0ld:%09ld\n", TimeUtils::diff(time3,time4).tv_sec, TimeUtils::diff(time3,time4).tv_nsec);
    printf("Write image:      %0ld:%09ld\n", TimeUtils::diff(time4,time5).tv_sec, TimeUtils::diff(time4,time5).tv_nsec);
    #endif
    eventRate.event();
    // TimeUtils::computeFramesPerSec();
  }
  if (cap) delete cap;
  if (record) delete record;
  return 0;
}
