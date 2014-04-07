#ifndef __IMAGE_CAPTURE_H__
#define __IMAGE_CAPTURE_H__

#include "Thread.h"

class OptionsProcess;
class ImageBufferManager;

/**
  @brief This class is used to aquire images from cameras and files.
*/
class ImageCapture : public Thread {
public:
  ImageCapture(OptionsProcess *optionsProcessIn, ImageBufferManager *imageBufferManagerIn);
  void init(void);
  void run(void);
  void stop(void);
  int camera;

 private:
  cv::VideoCapture *cap;
  cv::VideoCapture *cap0;
  cv::VideoCapture *cap1;
  cv::VideoCapture *cap2;
  OptionsProcess *options;
  ImageBufferManager *imageBufferManager;
  cv::Mat previousImage;
  bool stopped;
};

#endif
