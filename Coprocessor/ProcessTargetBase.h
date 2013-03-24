
#ifndef __PROCESS_TARGET_BASE__
#define __PROCESS_TARGET_BASE__

#include "TimeUtils.h"
#include <QGridLayout>

class ProcessTargetBase {
 public:
  ProcessTargetBase();
  virtual void processImage(cv::Mat &srcImage, cv::Mat &displayImage,
			    cv::Mat &finalImage, bool guiAll, EventRate &eventRate) = 0;
  virtual std::vector<std::string> &getImageNames() = 0;
  virtual void setImageDisplay(int index) = 0;
  virtual int getImageDisplay() = 0;
  virtual void writeDefines(FILE *file, std::string indent) {}
  virtual void showGui(QGridLayout *layout) {}
  virtual void hideGui(QGridLayout *layout) {}
};


#endif
