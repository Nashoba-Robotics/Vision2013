
#include "ui_MainWindow.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class ImageBufferManager;
class OptionsProcess;
class Process;

class MainWindow : public QMainWindow, private Ui::MainWindow {
  Q_OBJECT
  
public:
  MainWindow(ImageBufferManager *imageMgr, OptionsProcess *optionsProcess,
	     Process *process, QWidget *parent = 0);
  //     void setUrl(const QUrl &url);
protected:
  void paintEvent(QPaintEvent *pe);
  ImageBufferManager *imageBufferManager;
  OptionsProcess *optionsProcess;
  Process *process;
  QImage img;
  QTimer *timer;
  cv::Mat final;
  cv::Size imageSize;
public slots:
  //     void on_webView_loadFinished();
  void updateImage();
  void updateProcessingType(int type);
  void updateImageSize(int type);
  void updateImageType(int type);
  void updateFrameRate(int index);
  void updatePause(bool checked);
  void updateCamera(int cam);
  void writeImage();
  void writeDefines();
private:
  //     void examineChildElements(const QWebElement &parentElement,
  //                               QTreeWidgetItem *parentItem);
};
