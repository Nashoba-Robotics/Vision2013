
#ifndef __PROCESS_NO_TARGET__
#define __PROCESS_NO_TARGET__

#include "TimeUtils.h"

class ProcessNoTarget {
 public:
  class Images {
  public:
    cv::Mat final;
  };
  Images images;

  void initGui(bool guiAll);
  void processImage(cv::Mat &srcImage, cv::Mat &finalImage, bool guiAll, EventRate &eventRate);
  ProcessNoTarget();
};


#endif
