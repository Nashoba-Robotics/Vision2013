#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

/**
   @brief Utilities to manipulate time
*/
class TimeUtils {
 public:
  static timespec diff(timespec start, timespec end);
};

/**
   @brief Calculate the rate that events are happening
*/
class EventRate {
 public:
  double fps;                 //!< fps calculated using number of frames / seconds
  double avgFps;              //!< average fps
  double filterFps;           //!< low pass filtered fps
  int counter;                //!< frame counter
  double elapsed;             //!< floating point seconds elapsed since start
  timespec beginningTs;       //!< Initial time stamp
  timespec currentTs;         //!< Current time stamp
  timespec lastTs;            //!< Last time stamp
  void init();
  void event();
  EventRate();
};

#endif
