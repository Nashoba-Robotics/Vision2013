#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include "ImageCapture.h"
#include "OptionsProcess.h"
#include "ImageBufferManager.h"
#include <unistd.h>

ImageCapture::ImageCapture(OptionsProcess *optionsProcess, ImageBufferManager *imageBufferManagerIn) :
  options(optionsProcess),
  imageBufferManager(imageBufferManagerIn),
  camera(0),
  cap(0), cap0(0), cap1(0), cap2(0),
  stopped(false) {
}

void ImageCapture::stop() {
  stopped = true;
}

void ImageCapture::init(void) {
  // Process a video instead of a single image
  if (options->processVideo) {
    if (options->processVideoFile) {
      // Process a video file on disk
      cap = new cv::VideoCapture(options->fileName);
      if(!cap || !cap->isOpened()) { // check if we succeeded
	printf("ERROR: unable to open camera file: %s\n", options->fileName);
	exit(-1);
      }
      cap->set(CV_CAP_PROP_POS_AVI_RATIO, 1);
      printf("Frame count %f\n", cap->get(CV_CAP_PROP_POS_FRAMES));
      cap->set(CV_CAP_PROP_POS_AVI_RATIO, 0);
    } else {
      // Process a live camera
      
      //      cap = new cv::VideoCapture(CAMERA_HTTP_ADDR);
      switch (options->numCameras) {
      case 3:
	/*	cap2 = new cv::VideoCapture(2);
	if(!cap2 || !cap2->isOpened()) { // check if we succeeded
	  printf("ERROR: unable to open camera 2\n");
	  exit(-1);
	}
	// fallthrough
      case 2:
	cap1 = new cv::VideoCapture(1); 
	if(!cap1 || !cap1->isOpened()) { // check if we succeeded
	  printf("ERROR: unable to open camera 1\n");
	  exit(-1);
	}
	*/
	// fallthrough
      default:
	cap0 = new cv::VideoCapture(0); // open the default camera
	if(!cap0 || !cap0->isOpened()) { // check if we succeeded
	  printf("ERROR: unable to open camera 0\n");
	  exit(-1);
	}
      }
      
      cap = cap0;
      //      cap->set(CV_CAP_PROP_FPS, 125);
      //    cap->set(CV_CAP_PROP_FRAME_WIDTH, 640);
      //    cap->set(CV_CAP_PROP_FRAME_HEIGHT, 480);
    }
    {
      ImageBufferManager::Buffers buffers = imageBufferManager->captureBegin();
      cv::Mat &imageBuffer = imageBufferManager->getBuffer(buffers.source);
      *cap >> imageBuffer;
      imageBufferManager->captureComplete(buffers);
    }

  } else {
    // Load an image from a file

    ImageBufferManager::Buffers buffers = imageBufferManager->captureBegin();
    cv::Mat &imageBuffer = imageBufferManager->getBuffer(buffers.source);
    imageBuffer = cv::imread(options->fileName, 1 );
    imageBufferManager->captureComplete(buffers);
  }
}

void ImageCapture::run(void) {
  while (1) {
    if (stopped)
      break;
    //  printf("ImageCapture::run\n");
    if (options->processVideo) {
      ImageBufferManager::Buffers buffers = imageBufferManager->captureBegin();
      cv::Mat &imageBuffer = imageBufferManager->getBuffer(buffers.source);
      //      cap->set(CV_CAP_PROP_POS_AVI_RATIO, 1);
      if (!options->processVideoFile) {
	if (camera != options->getCamera()) {
	  delete cap;
	  camera = options->getCamera();
	  cap = new cv::VideoCapture(camera); // open the default camera
	  if(!cap || !cap->isOpened()) { // check if we succeeded
	    printf("ERROR: unable to open camera 0\n");
	    exit(-1);
	  }
	}
	  /*	switch (options->getCamera()) {
	case 0: cap = cap0; break;
	case 1: cap = cap1; break;
	case 2: cap = cap2; break;
	}
	  */
      }
      if (!options->pauseImage) {
	// If there are more images to process then process them else
	// return.  This will be the case when processing an image file.
	if (!cap->grab())
	  exit(0); // HACK ???
	//	return 0;
	if (!cap->retrieve(imageBuffer))
	  exit(0);  /// HACK 

	previousImage = imageBuffer;
      } else {
	imageBuffer = previousImage.clone();
      }
      if (options->processVideoFile) {
	//      printf("Frame count %f\n", cap->get(CV_CAP_PROP_POS_AVI_RATIO));
	usleep(options->videoFileFrameDelay);
      }
      imageBufferManager->captureComplete(buffers);
    }
  }
}
