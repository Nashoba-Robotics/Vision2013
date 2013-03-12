#ifndef __IMAGE_BUFFER_MANAGER_H__
#define __IMAGE_BUFFER_MANAGER_H__

#include <list>
#include "Semaphore.h"
#include "Mutex.h"

/**
   @brief Image Buffer
*/
class ImageBufferManager {
 public:
  static const int BUFFER_EMPTY = -1;
  static const int NUM_BUFS = 14;

  class Buffers {
  public:
    Buffers();
    void setBuffersEmpty(void);
    bool areEmpty(void);

    int source;
    int final;
  };
  
  cv::Mat buffers[NUM_BUFS];
  std::list<int> freeList;
  Buffers captureBuffers;
  Buffers processBuffers;
  Buffers displayBuffers;
  Mutex mutex;
  Semaphore captureSem;
  Semaphore processSem;
  Semaphore displaySem;

  ImageBufferManager();
  
  Buffers captureBegin();
  void captureComplete(Buffers buffer);
  Buffers processBegin();
  void processComplete(Buffers buffers);
  int displayAvailable();
  Buffers displayBegin();
  void displayComplete(Buffers buffers);
  cv::Mat &getBuffer(int buffer);
};

#endif
