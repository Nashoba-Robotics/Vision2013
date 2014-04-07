#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "OptionsProcess.h"
#include "TimeUtils.h"
#include "ImageUtils.h"
#include "Process.h"
#include "ImageBufferManager.h"
#include "ImageCapture.h"

#include <QApplication>
#include "MainWindow.h"

// Command line argument processing
OptionsProcess options;

ImageBufferManager imageBufferManager;

int main( int argc, char** argv ) {
  cv::VideoWriter *record = 0;
  
  // Process any opencv arguments
  QApplication app(argc, argv);
  cvInitSystem(argc, argv);
  options.processArgs(argc, argv);

  ImageCapture imageCapture(&options, &imageBufferManager);
  Process process(&options, &imageBufferManager);

  MainWindow mainwin(&imageBufferManager, &options, &process);
  mainwin.show();
  
  imageCapture.init();
  process.init();
  //  eventRate.init();

  imageCapture.startThread();
  process.startThread();

#if 0
  while (1) {
    timespec time1, time2, time3, time4, time5;

    clock_gettime(CLOCK_REALTIME, &time1);
    //    imageCapture.run();
    clock_gettime(CLOCK_REALTIME, &time2);
    //RE ADD    if (options.processVideo) {
    //      if (!pauseImage) {
    //	*record << imageBuffer;
    //      }
    //    }
    //    process.run();
    clock_gettime(CLOCK_REALTIME, &time3);

    char c = 0;
    if (options.processVideo) {
      static int counter = 0;
      counter++;
      //      if ((counter % 5) == 0)
      c = cv::waitKey(1);
    } else {
      // Non realtime images
      c = cv::waitKey(1);
    }
    //printf("loop\n");
    clock_gettime(CLOCK_REALTIME, &time4);

    if (c == 'q') return 0;

    clock_gettime(CLOCK_REALTIME, &time5);
    #ifdef DEBUG_PROCESSING_TIME
    printf("Retrieve:         %0ld:%09ld\n", TimeUtils::diff(time1,time2).tv_sec, TimeUtils::diff(time1,time2).tv_nsec);
    printf("Process callback: %0ld:%09ld\n", TimeUtils::diff(time2,time3).tv_sec, TimeUtils::diff(time2,time3).tv_nsec);
    printf("Wait:             %0ld:%09ld\n", TimeUtils::diff(time3,time4).tv_sec, TimeUtils::diff(time3,time4).tv_nsec);
    printf("Write image:      %0ld:%09ld\n", TimeUtils::diff(time4,time5).tv_sec, TimeUtils::diff(time4,time5).tv_nsec);
    #endif
    //    eventRate.event();
    // TimeUtils::computeFramesPerSec();
  }
#endif

  app.exec();
  imageCapture.stop();
  process.stop();
  imageCapture.waitThreadExit();
  process.waitThreadExit();
  sleep(1);
  
  if (record) delete record;
  return 0;
}
