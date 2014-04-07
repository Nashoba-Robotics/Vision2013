#include "opencv2/imgproc/imgproc.hpp"

#include <stdio.h>
#include <QTimer>
#include "MainWindow.h"
#include "ImageBufferManager.h"
#include "OptionsProcess.h"
#include "ImageUtils.h"
#include "Process.h"


MainWindow::MainWindow(ImageBufferManager *im, OptionsProcess *options, Process *processIn, QWidget *parent)
  : QMainWindow(parent), imageBufferManager(im), optionsProcess(options), process(processIn) {
  setupUi(this);
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(updateImage()));
  timer->start(33);
  processingTypeComboBox->addItem("None", OptionsProcess::targetTypeNone);
  processingTypeComboBox->addItem("Rect Target", OptionsProcess::targetTypeRect);
  processingTypeComboBox->addItem("Blue Pole", OptionsProcess::targetTypeBluePole);
  processingTypeComboBox->addItem("Red Pole", OptionsProcess::targetTypeRedPole);
  connect(processingTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateProcessingType(int)));
  processingTypeComboBox->setCurrentIndex(options->getProcessingType());

  imageSizeComboBox->addItem("640x480", 0);
  imageSizeComboBox->addItem("320x240", 1);
  imageSizeComboBox->addItem("160x120", 2);
  connect(imageSizeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImageSize(int)));
  imageSizeComboBox->setCurrentIndex(0);
  imageSize = cv::Size(640,480);


  displayRateComboBox->addItem("30 fps", 33);
  displayRateComboBox->addItem("15 fps", 66);
  displayRateComboBox->addItem("7.5 fps", 133);
  displayRateComboBox->addItem("5 fps", 200);
  displayRateComboBox->addItem("2 fps", 500);
  displayRateComboBox->addItem("1 fps", 1000);
  connect(displayRateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFrameRate(int)));
  //displayRateComboBox->setCurrentIndex(1);
  displayRateComboBox->setCurrentIndex(3);


  if (options->processVideoFile) {
    cameraComboBox->addItem("File", 0);
    movieSlider->setVisible(true);
  } else if (options->processJpegFile) {
    cameraComboBox->addItem("JPEG", 0);
    movieSlider->setVisible(false);
  } else {
    movieSlider->setVisible(false);
    for (int i = 0; i < options->numCameras; i++) {
      std::stringstream camera;
      camera << "Camera ";
      camera << i;
      cameraComboBox->addItem(camera.str().c_str(), i);
    }
    connect(cameraComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateCamera(int)));
  }
  action_Pause->setChecked(options->pauseImage);
  connect(action_Pause, SIGNAL(toggled(bool)), this, SLOT(updatePause(bool)));
  connect(action_WriteImage, SIGNAL(triggered()), this, SLOT(writeImage()));
  connect(action_WriteDefines, SIGNAL(triggered()), this, SLOT(writeDefines()));
}

void MainWindow::writeImage() {
  ImageUtils::writeImage(final);
}

void MainWindow::writeDefines() {
  FILE *file = fopen("VisionDefines.h", "w");
  if (process && file) {
    fprintf(file, "namespace Vision {\n");
    process->writeDefines(file, "  ");
    fprintf(file, "};\n");
  } else {
    printf("Could not open file for write defines\n");
  }
  fclose(file);
}

void MainWindow::updatePause(bool checked) {
  //  printf("Paused %d\n", checked);
  optionsProcess->pauseImage = checked;
}

void MainWindow::updateFrameRate(int index) {
  //  printf("Frame Rate %d\n", index);
  timer->start(displayRateComboBox->itemData(index).toInt());
  optionsProcess->videoFileFrameDelay = displayRateComboBox->itemData(index).toInt() * 1000;
}

void MainWindow::updateImageSize(int index) {
  //  printf("ImageSize %d\n", index);
  switch (index) {
  case 0: imageSize = cv::Size(640,480); break;
  case 1: imageSize = cv::Size(320,240); break;
  case 2: imageSize = cv::Size(160,120); break;
  default: printf("Illegal type updateImageSize\n"); exit(-1);
  }
}

void MainWindow::updateCamera(int cam) {
  //  printf("updateCamera %d\n", cam);
  optionsProcess->setCamera(cam);
}

void MainWindow::updateImageType(int type) {
  //  printf("updateImageType %d\n", type);
  ProcessTargetBase *processBase = process->getProcessing();
  processBase->setImageDisplay(type);
}

void MainWindow::updateProcessingType(int type) {
  //  printf("updateProcessingType %d\n", type);
  process->getProcessing()->hideGui(controlsGridLayout);
  optionsProcess->setProcessingType((OptionsProcess::TargetType) type);

  disconnect(imageTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImageType(int)));
  ProcessTargetBase *processBase = process->getProcessing();
  std::vector<std::string> imageStrs = processBase->getImageNames();
  imageTypeComboBox->clear();
  for (int i = 0; i < imageStrs.size(); i++) {
    imageTypeComboBox->addItem(imageStrs[i].c_str(), i);
  }
  int imageDisplay = processBase->getImageDisplay();
  connect(imageTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateImageType(int)));
  //  printf("ImageDisplay %d\n", imageDisplay);
  imageTypeComboBox->setCurrentIndex(processBase->getImageDisplay());
  processBase->showGui(controlsGridLayout);
}

void MainWindow::updateImage() {
  if (imageBufferManager->displayAvailable()) {
    ImageBufferManager::Buffers buffers = imageBufferManager->displayBegin();
    cv::Mat &image = imageBufferManager->getBuffer(buffers.display);
    final = image.clone();
    cv::Mat dest;
    //cvConvertImage(mat, image2Draw_mat, CV_CVTIMG_SWAP_RB);
    cv::resize(final, dest, imageSize);
    cv::cvtColor(dest, dest, CV_BGR2RGB);
    img = QImage((const unsigned char*)(dest.data), dest.cols, dest.rows, QImage::Format_RGB888);
    label->setPixmap(QPixmap::fromImage(img));
    imageBufferManager->displayComplete(buffers);
  }
}

void MainWindow::paintEvent(QPaintEvent *pe) {
  QMainWindow::paintEvent(pe);
}
