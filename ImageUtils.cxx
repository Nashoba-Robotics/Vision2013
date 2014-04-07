#include <stdio.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "ImageUtils.h"
#include <stdio.h>
using namespace cv;
using namespace std;


cv::Mat ImageUtils::convertToRGB(cv::Mat src) {
  cv::Mat ret;
  //  printf("Channels %d\n", src.channels());
  //  printf("Depth %d\n", src.depth());
  switch (src.channels()) {
  case 1: {
    Mat channels[] = {src, src, src};
    cv::merge(channels, 3, ret);
    break;
  }
  case 3: {
    ret = src;
    break;
  }
  default:
    printf("Unknown number of channels %d\n", src.channels());
  }
  return ret;
}


/**
   @brief     Calculate and display histogram of an image.
   @param[in] src  Source image matrix to compute the histogram on.
*/
void ImageUtils::calcHistogram(Mat &src, Mat &histImage) {
  // Separate the image in 3 places ( R, G and B )
  vector<Mat> rgb_planes;
  split( src, rgb_planes );
  
  // Establish the number of bins
  int histSize = 255;
  
  // Set the ranges ( for R,G,B) )
  float range[] = { 10, 255 } ;
  const float* histRange = { range };
  
  bool uniform = true; bool accumulate = false;
  
  Mat r_hist, g_hist, b_hist;
  
  // Compute the histograms:
  calcHist( &rgb_planes[0], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &rgb_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &rgb_planes[2], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
  
  // Draw the histograms for R, G and B
  int hist_w = 400; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

  histImage = Mat::zeros(src.size(), CV_8UC3);
  
  // Normalize the result to [ 0, histImage.rows ]
  normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  
  // Draw for each channel
  for( int i = 1; i < histSize; i++ ) {
    line(histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
	 Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
	 Scalar( 255, 0, 0), 2, 8, 0  );
    line(histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
	 Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
	 Scalar( 0, 255, 0), 2, 8, 0  );
    line(histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
	 Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
	 Scalar( 0, 0, 255), 2, 8, 0  );
  }
}

/**
   @brief Determine the intersection of two lines
   @param[in] line1Params Parameters describing line 1
   @param[in] line2Params Parameters describing line 2
   @param[out] targetQuads2f Intersecion point of the two lines
   @return true if the intersection was found, false otherwise
 */
bool ImageUtils::intersection(Vec4f &line1Params, Vec4f &line2Params, Point2f &targetQuads2f) {
  Point2f p1(line1Params[2], line1Params[3]);
  Point2f p3(line2Params[2], line2Params[3]);
  Point2f d1(line1Params[0], line1Params[1]);
  Point2f d3(line2Params[0], line2Params[1]);
  Point2f p2 = p1 + d1;
  Point2f p4 = p3 + d3;

  double cross = (p1.x - p2.x)*(p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x);
  if (abs(cross) < /*EPS*/1e-8) {
    printf("Bad line\n");
    return false;
  }

  //  double t1 = (x.x * d2.y - x.y * d2.x)/cross;
  //  targetQuads2f = o1 + d1 * t1;
  targetQuads2f.x = ((p1.x*p2.y - p1.y*p2.x) * (p3.x - p4.x) - (p1.x - p2.x) * (p3.x*p4.y - p3.y *p4.x))/cross;
  targetQuads2f.y = ((p1.x*p2.y - p1.y*p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x*p4.y - p3.y *p4.x))/cross;
  return true;
}

/**
   @brief      Given a quad and a hull, refine the corner points of a quad.
   @param[in]  targetQuads    A list of quads
   @param[in]  targetHulls    A list of hulls corresponding to the quad list
   @param[out] targetQuads2f  The refined list of quads (as floats)
   @param[out] targetQuads2fi The refined list of quads (as integers)
*/
void ImageUtils::refineCorners(vector<vector<Point> >&targetQuads,
		   vector<vector<Point> >&targetHulls,
		   vector<vector<Point2f> >&targetQuads2f,
		   vector<vector<Point> >&targetQuads2fi) {

  for (int i=0; i < targetQuads.size(); i++) {
    for (int j=0; j < targetQuads[i].size(); j++) {
      targetQuads2f[i].push_back(targetQuads[i][j]);
    }
    /*
    printf("TargetQuad ");
    for (int j=0; j < targetQuads[i].size(); j++) {
      printf("(%d,%d) ",targetQuads[i][j].x, targetQuads[i][j].y);
    }
    printf("\n");
    printf("Hull ");
    for (int j=0; j < targetHulls[i].size(); j++) {
      printf("(%d,%d) ",targetHulls[i][j].x, targetHulls[i][j].y);
    }
    printf("\n");
    */
#if 1
    vector<Point> line1;
    vector<Point> line2;
    vector<Point> line3;
    vector<Point> line4;
    Vec4f line1Params, line2Params, line3Params, line4Params;

    // If we were able to refine the line segments, then update the target quads
    // otherwise we will just use the unrefined target quads
    bool found = findLineSegments(targetQuads[i], targetHulls[i], line1, line2, line3, line4);
    if (found) {
      Point2f targetQuad0, targetQuad1, targetQuad2, targetQuad3;
      fitLine(line1, line1Params, CV_DIST_L2, 0, 0.01, 0.01);
      fitLine(line2, line2Params, CV_DIST_L2, 0, 0.01, 0.01);
      fitLine(line3, line3Params, CV_DIST_L2, 0, 0.01, 0.01);
      fitLine(line4, line4Params, CV_DIST_L2, 0, 0.01, 0.01);

      if (ImageUtils::intersection(line4Params, line1Params, targetQuad0) &&
	  ImageUtils::intersection(line1Params, line2Params, targetQuad1) &&
	  ImageUtils::intersection(line2Params, line3Params, targetQuad2) &&
	  ImageUtils::intersection(line3Params, line4Params, targetQuad3)) {
	// Assign values only if we are able to get all of them
	targetQuads2f[i][0] = targetQuad0;
	targetQuads2f[i][1] = targetQuad1;
	targetQuads2f[i][2] = targetQuad2;
	targetQuads2f[i][3] = targetQuad3;
      }
    }
#endif
    for (int j=0; j < targetQuads[i].size(); j++) {
      targetQuads2fi[i].push_back(targetQuads2f[i][j]);
    }
  }
}

/**
   @brief      Determine the lines that make up a quad in terms of the hulls points
   @param[in]  targetQuad  The quad used to determine the lines
   @param[in]  targetHull  The hull used to determine the lines
   @param[out] line1       1st line of the quad composed of points from the hull
   @param[out] line2       2nd line of the quad composed of points from the hull
   @param[out] line3       3rd line of the quad composed of points from the hull
   @param[out] line4       4th line of the quad composed of points from the hull
*/
bool ImageUtils::findLineSegments(vector<Point> &targetQuad, vector<Point> &targetHull,
				  vector<Point> &line1, vector<Point> &line2,
				  vector<Point> &line3, vector<Point> &line4) {
    // Find first element
    int k = 0;
    while (targetQuad[0] != targetHull[k]) {
      if (k > targetHull.size()) {
	//printf("Error - could not find target");
	return false;
      }
      k++;
    }
    rotate(targetHull.begin(), targetHull.begin()+k, targetHull.end());
  
    k = 0;
    while (targetQuad[1] != targetHull[k]) {
      line1.push_back(targetHull[k]);
      k++;
      if (k == targetHull.size()) {
	//printf("Error - could not find target");
	return false;
      }
    }
    line1.push_back(targetHull[k]);
    while (targetQuad[2] != targetHull[k]) {
      line2.push_back(targetHull[k]);
      k++;
      if (k == targetHull.size()) {
	//printf("Error - could not find target");
	return false;
      }
    }
    line2.push_back(targetHull[k]);
    while (targetQuad[3] != targetHull[k]) {
      line3.push_back(targetHull[k]);
      k++;
      if (k == targetHull.size()) {
	//printf("Error - could not find target");
	return false;
      }
    }
    line3.push_back(targetHull[k]);
    while (k < targetHull.size()) {

      line4.push_back(targetHull[k]);
      k++;
    }
    line4.push_back(targetHull[0]);
    return true;
}

/**
   @brief Determine if a polygon contains another polygon
     These are targets (Outer rectangle is the outer part of the reflective tape
     inner rectangle is the inner part of the reflective tape).
   @param[in] i  The polygon in prunedPoly to test if it contains another polygon
   @param[in] prunedPoly  A list of polygons to test against
   @return  true if the polygon contains another polygon, false otherwise
*/
bool ImageUtils::rectContainsRect(int i, const vector<vector<Point> > &prunedPoly) {
  for (int j =0; j < prunedPoly.size(); j++) {
    // Don't check against yourself
    if (i == j) continue;
    if (pointPolygonTest(prunedPoly[i], prunedPoly[j][0], false) > 0) {
      return true;
    }
  }
  return false;
}

std::string ImageUtils::getOutputVideoFileName(std::string name) {
  std::string outputName;
  char outputSuffix[100];
  time_t beginningTime;      // beginningTime and end timens
  tm *timeTm;
  time(&beginningTime);           // start the clock
  timeTm = localtime(&beginningTime);
  outputName = "RobotVideo";
  outputName += name;
  strftime(outputSuffix, sizeof(outputSuffix)-1, "_%Y_%m_%d_%H_%M_%S.mjpg", timeTm);
  outputName += outputSuffix;
  return outputName;
}

void ImageUtils::writeImage(Mat &src) {
  char outputFileName[100];
  time_t currentTime;
  tm *timeTm;
  time(&currentTime);
  timeTm = localtime(&currentTime);
  strftime(outputFileName, sizeof(outputFileName)-1, "RobotImage_%Y_%m_%d_%H_%M_%S.jpg", timeTm);
  imwrite(outputFileName, src);
}

void ImageUtils::rotatePt(cv::Point &pt, cv::Point originPt, double angle) {
  cv::Point temp;
  temp.x = (pt.x - originPt.x) * cos(angle) -
    (pt.y - originPt.y) * sin(angle) + originPt.x;
  temp.y = (pt.x - originPt.x) * sin(angle) +
    (pt.y - originPt.y) * cos(angle) + originPt.y;
  pt = temp;
}

void ImageUtils::rotatePts(cv::Point *pt, int numPts, cv::Point originPt, double angle) {
  for (int i = 0; i < numPts; i++) {
    rotatePt(pt[i], originPt, angle);
  }
}

void ImageUtils::translatePt(cv::Point &pt, cv::Point delta) {
  pt.x += delta.x;
  pt.y += delta.y;
}

void ImageUtils::translatePts(cv::Point *pt, int numPts, cv::Point delta) {
  for (int i = 0; i < numPts; i++) {
    translatePt(pt[i], delta);
  }
}
