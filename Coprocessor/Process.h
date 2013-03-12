#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "Thread.h"
#include "ProcessNoTarget.h"
#include "ProcessRectTarget.h"
#include "ProcessPoleTarget.h"
#include "TimeUtils.h"

class OptionsProcess;
class ImageBufferManager;

/**
  @brief This class is used to process images.
*/
class Process : public Thread {
public:
  Process(OptionsProcess *optionsProcessIn, ImageBufferManager *imageBufferManagerIn);
  void init(void);
  void run(void);
  void stop(void);

 private:
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
