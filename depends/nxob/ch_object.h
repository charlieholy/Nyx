#ifndef OBSERVER_H_
#define OBSERVER_H_

#include "ch_subject.h"

class Subject;
class Observer
{
public:
    virtual void Listen(Subject *aSubject) = 0;
    virtual ~Observer(){}
protected:
    Observer(){};
};



#endif /* OBSERVER_H_ */