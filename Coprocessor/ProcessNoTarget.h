
#ifndef __PROCESS_NO_TARGET__
#define __PROCESS_NO_TARGET__

#include "TimeUtils.h"
#include "ProcessTargetBase.h"

class ProcessNoTarget  : public ProcessTargetBase {
 public:
  class Images {
  public:
    cv::Mat final;
  };
  Images images;
  std::vector<std::string> imageNames;
  enum ImageIds {
    imageIdFinal
  };

  std::vector<std::string> &getImageNames() { return imageNames; }
  void setImageDisplay(int index) {}
  int getImageDisplay() {
    return imageIdFinal;
  }

  void initGui(bool guiAll);
  void processImage(cv::Mat &srcImage, cv::Mat &displayImage,
		    cv::Mat &finalImage, bool guiAll, EventRate &eventRate);
  ProcessNoTarget();
};


#endif
