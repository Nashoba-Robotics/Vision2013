#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <semaphore.h>

/**
   @brief Class wrapper for Semaphores
*/
class Semaphore {
 public:
  Semaphore(int value = 0);
  void acquire();
  void release();
  int available();
  
 private:
  sem_t sem;
};

#endif
