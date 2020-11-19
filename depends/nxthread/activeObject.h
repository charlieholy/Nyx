#ifndef __ACTIVE_OBJECT_H__
#define __ACTIVE_OBJECT_H__

#include <assert.h>
#include <list>
#include <string>
#include <time.h>
#include "ch_Lock.h"
#include <thread>

typedef struct _thread_job{
    std::shared_ptr<std::thread> __m_thread_in;
    bool __m_status = true;
}thread_job;
//Forward declare
class ActiveObject;

class ActiveObjectFactory
{
	public:
		typedef enum {
			Stopped, Running, Exitting
		}  ActiveObjectState_t;
		
    typedef struct 
    {
    	std::string  name;
    	ActiveObjectState_t state;
    	ActiveObject *obj;
    	
    	time_t lastUpdate;
    } ObjectInfo_t;

		static ActiveObjectFactory *Instance();
		
		template <typename T>
		T *createObject(const char *name)		{		  return new T(name);		}
		
		std::string getObjectInfo();
	private:
		friend class ActiveObject;
		int Register(ActiveObject *obj, std::string name);
		int unRegister(ActiveObject *obj);
		int setState(ActiveObject *obj, ActiveObjectState_t state);

    std::list<ObjectInfo_t> _objects;		
    static ActiveObjectFactory *_instance;
    CriticalLock _Lock;
} ;


class ActiveObject {
protected:
   ActiveObject(std::string name);
   virtual int run() = 0;
   virtual bool init() = 0;
public:
	ActiveObject() {};
   virtual ~ActiveObject();
   // 激活主动对象，开始执行run虚函数
   int active(bool isThread=true);
   int deactive(int join = 1);
   static void *ActiveObjectHelper(void *arg); 
   
private:
   	int       __m_Running;
   	ActiveObjectFactory* __m_Factory;
	std::string __m_name; 
protected:
	pthread_t _tid;
	
};

#endif
