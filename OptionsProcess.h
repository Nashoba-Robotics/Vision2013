#ifndef __OPTIONS_PROCESS_H__
#define __OPTIONS_PROCESS_H__

#include "Mutex.h"

/**
  @brief This class is used to process command line arguments and store
    options selected on the command line to be queried by other parts
    of the program.
*/
class OptionsProcess {
 public:
  typedef enum {
    targetTypeNone,
    targetTypeRect,
    targetTypeBluePole,
    targetTypeRedPole
  } TargetType;
  int processVideo;    //!< Indicates that we are processing a live camera image
  int processVideoFile; //!< Indicates that we are processing a video file
  int processJpegFile;  //!< Indicates that we are processing a jpeg image

  int guiAll;           //!< Show all debugging windows
  int verbose_flag;
  int processingType;
  bool pauseImage;
  int numCameras;
  int camera;
  char *fileName;
  int videoFileFrameDelay;
  int writeVideoSource;
  int writeVideoDisplay;
  int writeVideoFinal;
  void setProcessingType(TargetType type);
  TargetType getProcessingType();
  void setCamera(int cam);
  int getCamera();
 OptionsProcess(): processVideo(true), guiAll(false), processVideoFile(false),
    processJpegFile(false),
    fileName(0),
    pauseImage(false),
    numCameras(3),
    camera(0),
    writeVideoSource(0),
    writeVideoDisplay(0),
    writeVideoFinal(0),
    videoFileFrameDelay(33000) {
  }
  int processArgs(int argc, char** argv);
  Mutex mutex;
};

#endif
