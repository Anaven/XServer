/*
    File:       Thread.cpp

    Contains:   Thread abstraction implementation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "Thread.h"
#include "MyAssert.h"

//
// Thread.cpp
//
pthread_key_t Thread::gMainKey = 0;

void Thread::Initialize()
{
    pthread_key_create(&Thread::gMainKey, NULL);
}


Thread::Thread()
:   fStopRequested(false),
    fJoined(false),
    fThreadData(NULL)
{
}

Thread::~Thread()
{
    this->StopAndWaitForThread();
}

void Thread::Start()
{
    pthread_attr_t* theAttrP = NULL;

    int err = pthread_create((pthread_t*)&fThreadID, theAttrP, _Entry, (void*)this); // success 0
    Assert(err == 0);
}

void Thread::SetAffinity(UInt32 theMask)
{
    printf("Bind Cpu %u\n", theMask);
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(theMask, &cpuset);
    int err = pthread_setaffinity_np(fThreadID, sizeof(cpuset), &cpuset);
    Assert(err == 0);
}

void Thread::StopAndWaitForThread()
{
    fStopRequested = true;
    if (!fJoined)
        Join();
}

void Thread::Join()
{
    // What we're trying to do is allow the thread we want to delete to complete
    // running. So we wait for it to stop.
    Assert(!fJoined);
    fJoined = true;
    void *retVal;
    pthread_join((pthread_t)fThreadID, &retVal); 
}

void Thread::ThreadYield()
{
    // on platforms who's threading is not pre-emptive yield 
    // to another thread
    sched_yield();
}

void Thread::Sleep(UInt32 inMsec)
{
    if (inMsec == 0)
        return;
        
    if (inMsec < 1000)
        ::usleep(inMsec * 1000); // useconds must be less than 1,000,000
    else
        ::sleep((inMsec + 500) / 1000); // round to the nearest whole second
}


void* Thread::_Entry(void *inThread)  //static
{
    Thread* theThread = (Thread *)inThread;

    theThread->fThreadID = (pthread_t)pthread_self();
    pthread_setspecific(Thread::gMainKey, theThread);
    //
    // Run the thread
    theThread->Entry();

    return NULL;
}

Thread*   Thread::GetCurrent()
{
    return (Thread *)pthread_getspecific(Thread::gMainKey);
}

