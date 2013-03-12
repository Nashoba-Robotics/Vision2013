#include "opencv2/imgproc/imgproc.hpp"

#include <stdio.h>
#include <QTimer>
#include "MainWindow.h"
#include "ImageBufferManager.h"
#include "OptionsProcess.h"
#include "ImageUtils.h"


MainWindow::MainWindow(ImageBufferManager *im, OptionsProcess *options, QWidget *parent)
  : QMainWindow(parent), imageBufferManager(im), optionsProcess(options) {
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

  displayRateComboBox->addItem("30 fps", 33);
  displayRateComboBox->addItem("15 fps", 66);
  displayRateComboBox->addItem("7.5 fps", 133);
  displayRateComboBox->addItem("5 fps", 200);
  displayRateComboBox->addItem("2 fps", 500);
  displayRateComboBox->addItem("1 fps", 1000);
  connect(displayRateComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFrameRate(int)));
  displayRateComboBox->setCurrentIndex(0);

  if (options->processVideoFile || options->processJpegFile) {
    cameraComboBox->addItem("File", 0);
  } else {
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
}

void MainWindow::writeImage() {
  ImageUtils::writeImage(final);
}

void MainWindow::updatePause(bool checked) {
  printf("Paused %d\n", checked);
  optionsProcess->pauseImage = checked;
}

void MainWindow::updateFrameRate(int index) {
  printf("Frame Rate %d\n", index);
  timer->start(displayRateComboBox->itemData(index).toInt());
}

void MainWindow::updateCamera(int cam) {
  printf("updateCamera %d\n", cam);
  optionsProcess->setCamera(cam);
}

void MainWindow::updateProcessingType(int type) {
  printf("updateProcessingType %d\n", type);
  optionsProcess->setProcessingType((OptionsProcess::TargetType) type);
}

void MainWindow::updateImage() {
  if (imageBufferManager->displayAvailable()) {
        ImageBufferManager::Buffers buffers = imageBufferManager->displayBegin();
	cv::Mat &image = imageBufferManager->getBuffer(buffers.final);
	final = image.clone();
	cv::Mat dest;
	//cvConvertImage(mat, image2Draw_mat, CV_CVTIMG_SWAP_RB);
	cvtColor(image, dest, CV_BGR2RGB);
	img = QImage((const unsigned char*)(dest.data), dest.cols, dest.rows, QImage::Format_RGB888);
	label->setPixmap(QPixmap::fromImage(img));
    //    imshow("Final", finalImage);
    imageBufferManager->displayComplete(buffers);
  }
}

void MainWindow::paintEvent(QPaintEvent *pe) {
  if (imageBufferManager->displayAvailable()) {
        ImageBufferManager::Buffers buffers = imageBufferManager->displayBegin();
	cv::Mat &image = imageBufferManager->getBuffer(buffers.final);
	cv::Mat dest;
	//cvConvertImage(mat, image2Draw_mat, CV_CVTIMG_SWAP_RB);
	cvtColor(image, dest, CV_BGR2RGB);
	img = QImage((const unsigned char*)(dest.data), dest.cols, dest.rows, QImage::Format_RGB888);
	label->setPixmap(QPixmap::fromImage(img));
    //    imshow("Final", finalImage);
    imageBufferManager->displayComplete(buffers);
  }
  
  QMainWindow::paintEvent(pe);
}
