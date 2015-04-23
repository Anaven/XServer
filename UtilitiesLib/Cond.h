/*
    File:       Cond.h

    Contains:   A simple condition variable abstraction
    
    
        

*/

#ifndef _COND_H_
#define _COND_H_

#include "Headers.h"
#include "Mutex.h"
#include "MyAssert.h"
#include "Thread.h"

class Cond 
{
    public:

        Cond();
        ~Cond();
        
        inline void     Signal();
        inline void     Wait(Mutex* inMutex, SInt32 inTimeoutInMilSecs = 0);
        inline void     Broadcast();

    private:

        pthread_cond_t 	fCondition;
        void         	TimedWait(Mutex* inMutex, SInt32 inTimeoutInMilSecs);
};

inline void Cond::Wait(Mutex* inMutex, SInt32 inTimeoutInMilSecs)
{ 
    this->TimedWait(inMutex, inTimeoutInMilSecs);
}

inline void Cond::Signal()
{
    pthread_cond_signal(&fCondition);
}

inline void Cond::Broadcast()
{
    pthread_cond_broadcast(&fCondition);
}

#endif //_COND_H_

