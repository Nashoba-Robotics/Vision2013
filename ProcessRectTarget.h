
#ifndef __PROCESS_RECT_TARGET__
#define __PROCESS_RECT_TARGET__

#include "RectTarget.h"
#include "TimeUtils.h"
#include "ProcessTargetBase.h"
#include "HsvThresholdQt.h"

class ProcessRectTarget  : public ProcessTargetBase {
 public:
  class Images {
  public:
    cv::Mat grayScale;
    cv::Mat histImage;
    cv::Mat blurred;
    cv::Mat dilated;
    cv::Mat hsvImage;
    cv::Mat thresholded;
    cv::Mat contours;
    cv::Mat polygons;
    cv::Mat prunedPolygons;
    cv::Mat targets;
    cv::Mat prunedTargets;
    cv::Mat final;
    
    std::vector<cv::Mat> planes;
  };
  Images images;
  std::vector<std::string> imageNames;
  enum ImageId {
    imageIdGrayScale,
    imageIdHistogram,
    imageIdBlurred,
    imageIdDilated,
    imageIdHSV,
    imageIdThresholded,
    imageIdContours,
    imageIdPolygons,
    imageIdPrunedPolygons,
    imageIdTargets,
    imageIdPrunedTargets,
    imageIdFinal
  };
  ImageId imageId;

  virtual void showGui(QGridLayout *layout) {
    hsvThreshold.showGui(layout);
  }
  virtual void hideGui(QGridLayout *layout) {
    hsvThreshold.hideGui(layout);
  }
  virtual void writeDefines(FILE *file, std::string indent) {
    fprintf(file, "%s namespace %s {\n", indent.c_str(), "RectTarget");
    std::string indentMore = indent;
    indentMore += "  ";
    hsvThreshold.writeDefines(file, indentMore);
    fprintf(file, "%s };\n", indent.c_str());
  }

  std::vector<std::string> &getImageNames() { return imageNames; }
  int getImageDisplay() {
    return imageId;
  }
  void setImageDisplay(int index) {
    if ((index < imageIdGrayScale) || (index > imageIdFinal)) {
      imageId = imageIdFinal;
    } else {
      imageId = (ImageId)index;
    }
  }

  HsvThresholdQt hsvThreshold;
  
  class Params {
  public:
    int thresh;                   //!< Defines the threshold level to apply to image
    int thresh_block_size;        //!< Defines the threshold block size to apply to image

    int max_thresh_block_size;   //!< Defines the threshold block size to apply to image
    int max_thresh;              //!< Max threshold for the trackbar
    int poly_epsilon;             //!< Epsilon to determine reduce number of poly sides
    int max_poly_epsilon;         //!< Max epsilon for the trackbar
    
    int trackRed;
    int trackGreen;
    int trackBlue;
    double trackOffset;
    double trackMax;
    int max_colorsize;
    
    int minArea;                 //!< Polygons below this size are rejected
    int max_minArea;           //!< Max polygon size for the trackbar
    
    int dilation_elem;             //!< The element type dilate/erode with
    int max_elem;            //!< Max trackbar element type
    int dilation_size;             //!< The size of the element to dilate/erode with
    int max_kernel_size;    //!< Max trackbar kernel dialation size
    
    int erode_count;               //!< The number of times to erode the image
    int erode_max;                //!< Max number of times to erode on trackbar
    
    int target_width_inches;      //!< Width of a physical target in inches
    int target_height_inches;     //!< Height of a physical target in inches
  };
  Params params;
  
  void initGui(bool guiAll);
  void processImage(cv::Mat &srcImage, cv::Mat &displayImage, cv::Mat &finalImage,
		    bool guiAll, EventRate &eventRate);
  void targetsDebugText(cv::Mat &finalDrawing, RectTarget &target);
  ProcessRectTarget();
};


#endif
