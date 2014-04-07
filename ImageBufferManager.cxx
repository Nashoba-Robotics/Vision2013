#include <stdio.h>
#include <list>
#include "opencv2/imgproc/imgproc.hpp"
#include "Semaphore.h"
#include "ImageBufferManager.h"

ImageBufferManager::Buffers::Buffers() : source(BUFFER_EMPTY),
					 display(BUFFER_EMPTY),
					 final(BUFFER_EMPTY) {
}

void ImageBufferManager::Buffers::setBuffersEmpty(void) {
  source = BUFFER_EMPTY;
  display = BUFFER_EMPTY;
  final = BUFFER_EMPTY;
}

bool ImageBufferManager::Buffers::areEmpty(void) {
  assert( ((source == BUFFER_EMPTY) && (final == BUFFER_EMPTY) && (display == BUFFER_EMPTY)) ||
	  ((source != BUFFER_EMPTY) && (final != BUFFER_EMPTY) && (display != BUFFER_EMPTY)));
  return ((source == BUFFER_EMPTY) && (final == BUFFER_EMPTY) && (display == BUFFER_EMPTY));
}


ImageBufferManager::ImageBufferManager() : mutex() {
  for (int i =0; i < NUM_BUFS; i++) {
    freeList.push_back(i);
  }
}

cv::Mat &ImageBufferManager::getBuffer(int buffer) {
  assert((buffer >= 0) || (buffer < NUM_BUFS));

  return buffers[buffer];
}
  
ImageBufferManager::Buffers ImageBufferManager::captureBegin() {
  Buffers buffers;
  {
    mutex.lock();
    if (freeList.size() < 2) {
      assert(false);  // We ran out of buffers
    }
    buffers.source = freeList.front();
    freeList.pop_front();
    buffers.display = freeList.front();
    freeList.pop_front();
    buffers.final = freeList.front();
    freeList.pop_front();
    mutex.unlock();
  }
  return buffers;
}

void ImageBufferManager::captureComplete(Buffers buffers) {
  int existingResults = false;
  {
    mutex.lock();
    if (!captureBuffers.areEmpty()) {
      // Camera is faster than processing
      freeList.push_back(captureBuffers.source);
      freeList.push_back(captureBuffers.display);
      freeList.push_back(captureBuffers.final);
      existingResults = true;
      //      printf("Capture Buffer Not empty\n");
    }
    //    printf("Capture release Buffers %d %d %d\n", captureBuffers.source, captureBuffers.display, captureBuffers.final);
    captureBuffers = buffers;
    //    printf("Capture Buffers %d %d %d\n", buffers.source, buffers.display, buffers.final);
    // Release capture semaphore to let processing thread proceed
    if (!existingResults)
      captureSem.release();
    mutex.unlock();
  }
}

ImageBufferManager::Buffers ImageBufferManager::processBegin() {
  Buffers buffers;
  // Wait until we have a new capture buffer
  captureSem.acquire();
  {
    mutex.lock();
    buffers = captureBuffers;
    captureBuffers.setBuffersEmpty();
    mutex.unlock();
  }
  return buffers;
}

void ImageBufferManager::processComplete(Buffers buffers) {
  int existingResults = false;
  {
    mutex.lock();
    if (!processBuffers.areEmpty()) {
      // Processing faster than displaying
      freeList.push_back(processBuffers.source);
      freeList.push_back(processBuffers.display);
      freeList.push_back(processBuffers.final);
      existingResults = true;
      //      printf("Process Buffer Not empty\n");
    }
    processBuffers = buffers;
    //    printf("Processing complete Buffers %d %d %d\n", buffers.source, buffers.display, buffers.final);

    // Release processing semaphore to let the displaying thread proceed
    if (!existingResults)
      processSem.release();

    mutex.unlock();
  }
  
}

int ImageBufferManager::displayAvailable() {
  return processSem.available();
}

ImageBufferManager::Buffers ImageBufferManager::displayBegin() {
  Buffers buffers;
  // Wait until we have a new capture buffer
  processSem.acquire();
  {
    mutex.lock();
    buffers = processBuffers;
    processBuffers.setBuffersEmpty();
    mutex.unlock();
  }
  return buffers;
}

void ImageBufferManager::displayComplete(Buffers buffers) {
  int existingResults = false;
  {
    mutex.lock();
    if (!displayBuffers.areEmpty()) {
      // Processing faster than displaying
      freeList.push_back(displayBuffers.source);
      freeList.push_back(displayBuffers.display);
      freeList.push_back(displayBuffers.final);
      existingResults = true;
    }
    displayBuffers = buffers;
  
    // Release processing semaphore to let the displaying thread proceed
    if (!existingResults)
      displaySem.release();
    
    mutex.unlock();
  }
}
