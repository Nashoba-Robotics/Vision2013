#ifndef __ARGUMENT_H__
#define __ARGUMENT_H__

// Image Plane definitions
extern int BLUE_PLANE;
extern int GREEN_PLANE;
extern int RED_PLANE;

/**
  @brief This class is used to process command line arguments and store
    options selected on the command line to be queried by other parts
    of the program.
*/
class OptionsProcess {
 public:
  int processCamera;    //!< Indicates that we are processing a live camera image
  int processVideoFile; //!< Indicates that we are processing a video file
  int processJpegFile;  //!< Indicates that we are processing a jpeg image

  int guiAll;           //!< Show all debugging windows
  int verbose_flag;
  char *fileName;
 OptionsProcess(): processCamera(true), guiAll(false), processVideoFile(false),
    processJpegFile(false),
    fileName(0) {
  }
  int processArgs(int argc, char** argv);
};

#endif
