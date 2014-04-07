#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>

class Thread
{
public:
   Thread() {}
   virtual ~Thread() {}

   bool startThread()
   {
      return (pthread_create(&thread, NULL, staticThreadFunc, this) == 0);
   }
   virtual void quit(void) {
     pthread_exit(NULL);
   }
     
   void waitThreadExit()
   {
      (void) pthread_join(thread, NULL);
   }

   virtual void run() = 0;

private:
   static void * staticThreadFunc(void * thisPtr) {
     ((Thread *)thisPtr)->run();
     return NULL;
   }

   pthread_t thread;
};

#endif
