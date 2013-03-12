#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Defines.h"
#include "Messaging.h"
#include "ImageUtils.h"
#include "ProcessNoTarget.h"

using namespace cv;
using namespace std;

// This is called every time that we get a new image to process the image, get target data,
// and send the information to the crio
void ProcessNoTarget::processImage(Mat &srcImage, Mat &finalImage, bool guiAll,
				     EventRate &eventRate) {
  finalImage = srcImage.clone();

#ifdef DEBUG_TEXT
  Point fpsAlign( 10, 10 );
  ostringstream fpsText;
  fpsText.precision(3);
  fpsText << "Avg FPS=" << eventRate.avgFps << " Filt FPS=" << eventRate.filterFps << " FPS="<< eventRate.fps ;
  putText( finalImage, fpsText.str(), fpsAlign, CV_FONT_HERSHEY_PLAIN, .7, ColorWhite );
#endif

  //    sendMessagePole(CRIO_IP_ADDR, bestTarget.distanceY, bestTarget.angleX, bestTarget.anglePole);
  
  //  imshow("Final", finalImage);
}

ProcessNoTarget::ProcessNoTarget() {
}

void ProcessNoTarget::initGui(bool guiAll) {
  /// Create Window
  if (guiAll) {
    namedWindow("Final", CV_WINDOW_AUTOSIZE | CV_GUI_EXPANDED);
  }
}

