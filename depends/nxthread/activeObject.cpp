#include <assert.h>
#include <map>
#include <string>
#include <sstream>
#include "activeObject.h"
#include "ch_log.h"

ActiveObjectFactory * ActiveObjectFactory::_instance = NULL;

ActiveObjectFactory *ActiveObjectFactory::Instance()
{
	if(_instance == NULL)
		_instance = new ActiveObjectFactory();
		
	return _instance;
}

int ActiveObjectFactory::Register(ActiveObject *obj, std::string name)
{
	ObjectInfo_t info;

	info.name = name;
	info.state= Stopped;
	info.obj  = obj;
	
	time(&info.lastUpdate);
	
	_Lock.Lock();
	_objects.push_back(info);
	_Lock.Unlock();
	
	return 0;
}

int ActiveObjectFactory::unRegister(ActiveObject *obj)
{
    std::list<ObjectInfo_t>::iterator iter;		
	
	  _Lock.Lock();
	  for(iter = _objects.begin(); iter != _objects.end(); iter ++)
	  {
	  	  if(iter->obj == obj)
	  	  {
	  	      _objects.erase(iter);
	  	      break;
	  	  }
	  }
  	
	  _Lock.Unlock();
	  return 0;
}

int ActiveObjectFactory::setState(ActiveObject *obj, ActiveObjectState_t state)
{
    std::list<ObjectInfo_t>::iterator iter;		

		//fprintf(stdout, " +setState\n");
	  _Lock.Lock();
	  for(iter = _objects.begin(); iter != _objects.end(); iter ++)
	  {
	  	  if(iter->obj == obj)
	  	  {
					  iter->state = state;
						time(&iter->lastUpdate);
	  	      break;
	  	  }
	  }
	  _Lock.Unlock();

	  	//fprintf(stdout, " -setState\n");
	  return 0;
}
		
std::string ActiveObjectFactory::getObjectInfo()
{
    std::list<ObjectInfo_t>::iterator iter;		
    std::ostringstream ostr;	  	
    
	  for(iter = _objects.begin(); iter != _objects.end(); iter ++)
	  {
	  	  ostr << "name=" << iter->name << "\tobj=" << (unsigned long)iter->obj << "\tstate=" << iter->state << std::endl;
	  }
	  
	  return ostr.str();
}

ActiveObject::ActiveObject(std::string name)
	:__m_Factory(ActiveObjectFactory::Instance()),__m_name(name)
{	
	
}

ActiveObject::~ActiveObject()
{
	__m_Factory->unRegister(this);
}

int ActiveObject::active(bool isThread)
{
	if(!init())
	{	
		L_INFO("============= {} init error =============\n",__m_name);
		return -1;
	}
	L_INFO("============= {} init ok =============\n",__m_name);
	if(!isThread){
		return 0;
	}
	__m_Factory->Register(this, __m_name);

    int err;
	
	__m_Running = 1;
	  
	 err = pthread_create(&_tid, NULL, ActiveObjectHelper, this);

	//fprintf(stdout, " active : pthread_create %d\n",err);
	
  	if(err)
	  {
		    __m_Running = 0;
	  } else {
    	  __m_Factory->setState(this, ActiveObjectFactory::Running);
    }

  	//fprintf(stdout, " -active\n");
	
    return err ? -1 : 0;		
}


int ActiveObject::deactive(int join )
{
    if(__m_Running==0){
		return -1;
	}
	
	  __m_Running = 0;
	  	
	  __m_Factory->setState(this, ActiveObjectFactory::Exitting);

	  if(join) pthread_join(_tid, NULL);

	  return 0;
}


void *ActiveObject::ActiveObjectHelper(void * arg)
{
	  ActiveObject *obj = (ActiveObject *)arg;

	 //fprintf(stdout, " ActiveObjectHelper\n");
	  while(obj->__m_Running)
	  {	
	      obj->__m_Factory->setState(obj, ActiveObjectFactory::Running);
	      obj->run();
	  }
	  
      obj->__m_Factory->setState(obj, ActiveObjectFactory::Stopped);
	  
	  return NULL;
}
