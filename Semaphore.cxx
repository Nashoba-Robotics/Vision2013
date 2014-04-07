#include "Semaphore.h"

Semaphore::Semaphore(int value) {
  sem_init(&sem, 0, value);
}

void Semaphore::acquire() {
  sem_wait(&sem);
}

void Semaphore::release() {
  sem_post(&sem);
}

int Semaphore::available() {
  int value;
  sem_getvalue(&sem, &value);
  return value;
}
