#include <stdio.h>
#include <time.h>
#include "TimeUtils.h"

/**
  @brief Compute the difference between two timespec's i.e.
     difference = end - start
  @param[in] start  Starting timestamp
  @param[in] end    Ending timestamp
  @return    difference between start and end
*/
timespec TimeUtils::diff(timespec start, timespec end)
{
    timespec temp;
    // If the number of ns rolls, we need to bump the number of sec
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

/**
  @brief Default constructor
*/
EventRate::EventRate(): fps(0), avgFps(0), counter(0), elapsed(0) {
}

/**
 @brief Initialize the EventRate class
*/
void EventRate::init() {
  fps = 0;
  avgFps = 0;
  elapsed = 0;
  clock_gettime(CLOCK_REALTIME, &beginningTs);
  clock_gettime(CLOCK_REALTIME, &lastTs);
  clock_gettime(CLOCK_REALTIME, &currentTs);
}

/**
  @brief Compute the difference between this event and the last one.  The first
         event is initialized by init().
*/
void EventRate::event() {
 // see how much time has elapsed
  clock_gettime(CLOCK_REALTIME, &currentTs);
    
  // calculate current FPS
  ++counter;       
  elapsed = (currentTs.tv_nsec - beginningTs.tv_nsec)*1e-9+ (currentTs.tv_sec - beginningTs.tv_sec);     
  avgFps = counter / elapsed;
  elapsed = (currentTs.tv_nsec - lastTs.tv_nsec)*1e-9+ (currentTs.tv_sec - lastTs.tv_sec);     
  fps = 1/elapsed;
  const double weight = .01;
  filterFps = weight * fps + (1 - weight) * filterFps;
  // will print out Inf until sec is greater than 0
  //  printf("Avg FPS = %.2f. FPS = %.2f\n", avgFps, fps);
  lastTs = currentTs;
}



