/*
    File:       Mutex.h

    Contains:   

    

*/

#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/errno.h>

#include "Headers.h"
#include "MyAssert.h"
#include "Thread.h"

class Cond;

class Mutex
{
    public:

        Mutex();
        ~Mutex();

        inline void Lock();
        inline void Unlock();
        
        // Returns true on successful grab of the lock, false on failure
        inline bool TryLock();

    private:

        pthread_mutex_t fMutex;
        // These two platforms don't implement pthreads recursive mutexes, so
        // we have to do it manually
        pthread_t   fHolder;
        UInt32		fHolderCount;
      
        void		RecursiveLock();
        void		RecursiveUnlock();
        bool		RecursiveTryLock();

        friend class Cond;
};

class   MutexLocker
{
    public:

        MutexLocker(Mutex *inMutexP) : fMutex(inMutexP) { if (fMutex != NULL) fMutex->Lock(); }
        ~MutexLocker() {  if (fMutex != NULL) fMutex->Unlock(); }
        
        void Lock()         { if (fMutex != NULL) fMutex->Lock(); }
        void Unlock()       { if (fMutex != NULL) fMutex->Unlock(); }
        
    private:

        Mutex*    fMutex;
};

void Mutex::Lock()
{
    this->RecursiveLock();
}

void Mutex::Unlock()
{
    this->RecursiveUnlock();
}

bool Mutex::TryLock()
{
    return this->RecursiveTryLock();
}

#endif //_OSMUTEX_H_
