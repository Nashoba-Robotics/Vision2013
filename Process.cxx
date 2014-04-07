#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include "Process.h"
#include "OptionsProcess.h"
#include "ImageBufferManager.h"
#include "ImageUtils.h"

Process::Process(OptionsProcess *optionsProcess, ImageBufferManager *imageBufferManagerIn) :
  options(optionsProcess),
  imageBufferManager(imageBufferManagerIn),
  processNoTarget(),
  processRectTarget(),
  processBluePoleTarget(ProcessPoleTarget::BlueTarget),
  processRedPoleTarget(ProcessPoleTarget::RedTarget),
  stopped(false),
  recordSource(0),
  recordDisplay(0),
  recordFinal(0) {
}

void Process::writeDefines(FILE *file, std::string indent) {
  processNoTarget.writeDefines(file, indent);
  processRectTarget.writeDefines(file, indent);
  processBluePoleTarget.writeDefines(file, indent);
  processRedPoleTarget.writeDefines(file, indent);
}

void Process::writeVideo(cv::Mat &srcImage, cv::Mat &displayImage, cv::Mat &finalImage) {
  if (options->writeVideoSource) {
    if (!recordSource) {
      std::string filename = ImageUtils::getOutputVideoFileName("Source");
      recordSource = new cv::VideoWriter(filename.c_str(), CV_FOURCC('M', 'J', 'P', 'G'), 30, srcImage.size(), true);
      if (!recordSource->isOpened()) {
	printf("VideoWriter failed to open!\n");
	exit(-1);
      }
    }
    *recordSource << srcImage;
  }
  if (options->writeVideoDisplay) {
    if (!recordDisplay) {
      std::string filename = ImageUtils::getOutputVideoFileName("Display");
      recordDisplay =  new cv::VideoWriter(filename.c_str(), CV_FOURCC('M', 'J', 'P', 'G'), 30, displayImage.size(), true);
      if (!recordDisplay->isOpened()) {
	printf("VideoWriter failed to open!\n");
	exit(-1);
      }
    }
    *recordDisplay << displayImage;
  }
  if (options->writeVideoFinal) {
    if (!recordFinal) {
      std::string filename = ImageUtils::getOutputVideoFileName("Final");
      recordFinal =  new cv::VideoWriter(filename.c_str(), CV_FOURCC('M', 'J', 'P', 'G'), 30, finalImage.size(), true);
      if (!recordFinal->isOpened()) {
	printf("VideoWriter failed to open!\n");
	exit(-1);
      }
    }
    *recordFinal << finalImage;
  }
}


void Process::stop() {
  stopped = true;
}

void Process::init(void) {
  eventRate.init();
  switch (options->getProcessingType()) {
  case OptionsProcess::targetTypeNone:
    processNoTarget.initGui(options->guiAll);
    break;
  case OptionsProcess::targetTypeRect:
    processRectTarget.initGui(options->guiAll);
    break;
  case OptionsProcess::targetTypeBluePole:
    processBluePoleTarget.initGui(options->guiAll);
    break;
  case OptionsProcess::targetTypeRedPole:
    processRedPoleTarget.initGui(options->guiAll);
    break;
  default:
    printf("Illegal processing Type\n");
    exit(-1);
  }
  
}

ProcessTargetBase *Process::getProcessing(void) {
    switch (options->processingType) {
    case OptionsProcess::targetTypeNone:
      return &processNoTarget;
    case OptionsProcess::targetTypeRect:
      return &processRectTarget;
    case OptionsProcess::targetTypeBluePole:
      return &processBluePoleTarget;
    case OptionsProcess::targetTypeRedPole:
      return &processRedPoleTarget;
    default:
      printf("Illegal processing Type\n");
      exit(-1);
    }
    return NULL;
}

void Process::run(void) {
  //  printf("Process::run\n");
  while (1) {
    if (stopped) {
      //printf("Saving Video\n");
      if (recordSource) delete recordSource;
      if (recordDisplay) delete recordDisplay;
      if (recordFinal) delete recordFinal;
      break;
    }
    ImageBufferManager::Buffers buffers = imageBufferManager->processBegin();
    cv::Mat &sourceImage = imageBufferManager->getBuffer(buffers.source);
    cv::Mat &displayImage = imageBufferManager->getBuffer(buffers.display);
    cv::Mat &finalImage = imageBufferManager->getBuffer(buffers.final);

    getProcessing()->processImage(sourceImage, displayImage, finalImage, options->guiAll, eventRate);
    writeVideo(sourceImage, displayImage, finalImage);
    imageBufferManager->processComplete(buffers);
    eventRate.event();
  }
}

