#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>

/**
   @brief Class wrapper for Mutex
*/
class Mutex {
 public:
  Mutex();
  void lock();
  void unlock();
  
 private:
  pthread_mutex_t mutex;
};

#endif

