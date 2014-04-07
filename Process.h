#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "Thread.h"
#include "ProcessNoTarget.h"
#include "ProcessRectTarget.h"
#include "ProcessPoleTarget.h"
#include "TimeUtils.h"

class OptionsProcess;
class ImageBufferManager;
class ProcessTargetBase;

/**
  @brief This class is used to process images.
*/
class Process : public Thread {
public:
  Process(OptionsProcess *optionsProcessIn, ImageBufferManager *imageBufferManagerIn);
  void init(void);
  void run(void);
  void stop(void);
  ProcessTargetBase *getProcessing(void);
  void writeDefines(FILE *file, std::string indent);
  void writeVideo(cv::Mat &srcImage, cv::Mat &displayImage, cv::Mat &finalImage);


 private:
  cv::VideoWriter *recordSource;
  cv::VideoWriter *recordDisplay;
  cv::VideoWriter *recordFinal;

  OptionsProcess *options;
  ImageBufferManager *imageBufferManager;
  ProcessNoTarget processNoTarget;
  ProcessRectTarget processRectTarget;
  ProcessPoleTarget processBluePoleTarget;
  ProcessPoleTarget processRedPoleTarget;
  EventRate eventRate;
  bool stopped;
};

#endif
