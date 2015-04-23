#include "MutexRW.h"
#include "Mutex.h"
#include "Cond.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
   
void MutexRW::LockRead()
{
    MutexLocker locker(&fInternalLock);
    
    AddReadWaiter();
    while   (   ActiveWriter() // active writer so wait
            ||  WaitingWriters() // reader must wait for write waiters
            )
    {   
        fReadersCond.Wait(&fInternalLock, MutexRW::eMaxWait);
    }
        
    RemoveReadWaiter();
    AddActiveReader(); // add 1 to active readers
    fActiveReaders = fState;
}

void MutexRW::LockWrite()
{
    MutexLocker locker(&fInternalLock);
    AddWriteWaiter();       //  1 writer queued            

    while   (ActiveReaders())  // active readers
    {       
        fWritersCond.Wait(&fInternalLock, MutexRW::eMaxWait);
    }

    RemoveWriteWaiter(); // remove from waiting writers
    SetState(MutexRW::eActiveWriterState);    // this is the active writer    
    fActiveReaders = fState; 
}


void MutexRW::Unlock()
{           
    MutexLocker locker(&fInternalLock);

    if (ActiveWriter()) 
    {           
        SetState(MutexRW::eNoWriterState); // this was the active writer 
        if (WaitingWriters()) // there are waiting writers
        {   fWritersCond.Signal();
        }
        else
        {   fReadersCond.Broadcast();
        }
    }
    else
    {
        RemoveActiveReader(); // this was a reader
        if (!ActiveReaders()) // no active readers
        {   SetState(MutexRW::eNoWriterState); // this was the active writer now no actives threads
            fWritersCond.Signal();
        } 
    }
    fActiveReaders = fState;
}



// Returns true on successful grab of the lock, false on failure
int MutexRW::TryLockWrite()
{
    int    status  = EBUSY;
    MutexLocker locker(&fInternalLock);

    if ( !Active() && !WaitingWriters()) // no writers, no readers, no waiting writers
    {
        this->LockWrite();
        status = 0;
    }

    return status;
}

int MutexRW::TryLockRead()
{
    int    status  = EBUSY;
    MutexLocker locker(&fInternalLock);

    if ( !ActiveWriter() && !WaitingWriters() ) // no current writers but other readers ok
    {
        this->LockRead(); 
        status = 0;
    }
    
    return status;
}

