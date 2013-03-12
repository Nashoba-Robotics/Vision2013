#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include "Process.h"
#include "OptionsProcess.h"
#include "ImageBufferManager.h"

Process::Process(OptionsProcess *optionsProcess, ImageBufferManager *imageBufferManagerIn) :
  options(optionsProcess),
  imageBufferManager(imageBufferManagerIn),
  processNoTarget(),
  processRectTarget(),
  processBluePoleTarget(ProcessPoleTarget::BlueTarget),
  processRedPoleTarget(ProcessPoleTarget::RedTarget),
  stopped(false) {
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

void Process::run(void) {
  //  printf("Process::run\n");
  while (1) {
    if (stopped)
      break;
    ImageBufferManager::Buffers buffers = imageBufferManager->processBegin();
    cv::Mat &sourceImage = imageBufferManager->getBuffer(buffers.source);
    cv::Mat &finalImage = imageBufferManager->getBuffer(buffers.final);
    
    switch (options->processingType) {
    case OptionsProcess::targetTypeNone:
      processNoTarget.processImage(sourceImage, finalImage, options->guiAll, eventRate);
      break;
    case OptionsProcess::targetTypeRect:
      processRectTarget.processImage(sourceImage, finalImage, options->guiAll, eventRate);
      break;
    case OptionsProcess::targetTypeBluePole:
      processBluePoleTarget.processImage(sourceImage, finalImage, options->guiAll, eventRate);
      break;
    case OptionsProcess::targetTypeRedPole:
      processRedPoleTarget.processImage(sourceImage, finalImage, options->guiAll, eventRate);
      break;
    default:
      printf("Illegal processing Type\n");
      exit(-1);
    }
    imageBufferManager->processComplete(buffers);
    eventRate.event();
  }
}

