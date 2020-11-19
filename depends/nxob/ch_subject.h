#ifndef SUBJECT_H_
#define SUBJECT_H_

#include "ch_object.h"

class Observer;
class Subject
{
public:
    virtual ~Subject(){}
    virtual void Attach(Observer *aObserver) = 0;
    virtual void EventNotice() = 0;
	
protected:
    Subject(){};

};

#endif /* SUBJECT_H_ */