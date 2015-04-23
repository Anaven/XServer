/*
    File:       Mutex.cpp

    Contains:   

    

*/

#include "Mutex.h"
#include <stdlib.h>
#include <string.h>

static pthread_mutexattr_t  *sMutexAttr=NULL;
static void MutexAttrInit();

static pthread_once_t sMutexAttrInit = PTHREAD_ONCE_INIT;

Mutex::Mutex()
{
    (void)pthread_once(&sMutexAttrInit, MutexAttrInit);
    (void)pthread_mutex_init(&fMutex, sMutexAttr);
    
    fHolder = 0;
    fHolderCount = 0;
}

void MutexAttrInit()
{
    sMutexAttr = (pthread_mutexattr_t*)malloc(sizeof(pthread_mutexattr_t));
    ::memset(sMutexAttr, 0, sizeof(pthread_mutexattr_t));
    pthread_mutexattr_init(sMutexAttr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&fMutex);
}

void Mutex::RecursiveLock()
{
    // We already have this mutex. Just refcount and return
    if (Thread::GetCurrentThreadID() == fHolder)
    {
        fHolderCount++;
        return;
    }

    (void)pthread_mutex_lock(&fMutex);

    Assert(fHolder == 0);
    fHolder = Thread::GetCurrentThreadID();
    fHolderCount++;
    Assert(fHolderCount == 1);
}

void Mutex::RecursiveUnlock()
{
    if (Thread::GetCurrentThreadID() != fHolder)
        return;
        
    Assert(fHolderCount > 0);
    fHolderCount--;
    if (fHolderCount == 0)
    {
        fHolder = 0;
        pthread_mutex_unlock(&fMutex);
    }
}

bool Mutex::RecursiveTryLock()
{
    // We already have this mutex. Just refcount and return
    if (Thread::GetCurrentThreadID() == fHolder)
    {
        fHolderCount++;
        return true;
    }

    int theErr = pthread_mutex_trylock(&fMutex);
    if (theErr != 0)
    {
        Assert(theErr == EBUSY);
        return false;
    }

    Assert(fHolder == 0);
    fHolder = Thread::GetCurrentThreadID();
    fHolderCount++;
    Assert(fHolderCount == 1);
    return true;
}
