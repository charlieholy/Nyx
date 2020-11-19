#ifndef CRITICALCOND_H
#define CRITICALCOND_H

#include <pthread.h>

class CriticalCond:public CriticalLock
{

public:
      pthread_cond_t  _Cond;
    
	  CriticalCond()  
	  {   
 	  	  pthread_cond_init(&_Cond,NULL);
	  }
	  ~CriticalCond() {pthread_cond_destroy(&_Cond);}
	  
	  void Signal()   { pthread_cond_signal(&_Cond);   }
	  void Wait() 
	  { 
	  	Lock();
		pthread_cond_wait(&_Cond,&_Mutex);
		Unlock();
	  }
};




#endif
