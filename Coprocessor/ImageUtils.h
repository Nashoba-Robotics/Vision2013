#ifndef __IMAGEUTILS_H__
#define __IMAGEUTILS_H__

/**
   @brief Utilities for working with images.
*/
class ImageUtils {
 public:
  static cv::Mat convertToRGB(cv::Mat src);
  static void calcHistogram(cv::Mat &src, cv::Mat &histImage);
  static bool intersection(cv::Vec4f &line1Params, cv::Vec4f &line2Params, cv::Point2f &targetQuads2f);
  static void refineCorners(std::vector<std::vector<cv::Point> >&targetQuads,
			    std::vector<std::vector<cv::Point> >&targetHulls,
			    std::vector<std::vector<cv::Point2f> >&targetQuads2f,
			    std::vector<std::vector<cv::Point> >&targetQuads2fi);
  static bool findLineSegments(std::vector<cv::Point> &targetQuad,
			       std::vector<cv::Point> &targetHull,
			       std::vector<cv::Point> &line1,
			       std::vector<cv::Point> &line2,
			       std::vector<cv::Point> &line3,
			       std::vector<cv::Point> &line4);
  static void rotatePt(cv::Point &pt, cv::Point origin, double angle);
  static void rotatePts(cv::Point *pt, int numPts, cv::Point originPt, double angle);
  static void translatePt(cv::Point &pt, cv::Point delta);
  static void translatePts(cv::Point *pt, int numPts, cv::Point delta);
  static bool rectContainsRect(int i, const std::vector<std::vector<cv::Point> > &prunedPoly);
  static std::string getOutputVideoFileName(std::string name);
  static void writeImage(cv::Mat &src);
};

// Image Plane definitions
#define BLUE_PLANE  0
#define GREEN_PLANE 1
#define RED_PLANE   2

static const cv::Scalar ColorWhite   = cv::Scalar( 255, 255, 255 );
static const cv::Scalar ColorPurple  = cv::Scalar( 255, 0, 255 );
static const cv::Scalar ColorGray    = cv::Scalar( 64, 64, 64 );
static const cv::Scalar ColorBlue    = cv::Scalar( 255, 0, 0 );
static const cv::Scalar ColorGreen   = cv::Scalar( 0, 255, 0 );
static const cv::Scalar ColorRed     = cv::Scalar( 0, 0, 255 );
static const cv::Scalar ColorYellow  = cv::Scalar( 0, 255, 255 );

#endif
