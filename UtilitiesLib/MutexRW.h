#ifndef _MutexRW_H_
#define _MutexRW_H_

#include <stdlib.h>
#include "Thread.h"
#include "MyAssert.h"
#include "Mutex.h"
#include "Queue.h"

#define DEBUGMUTEXRW 0

class MutexRW
{
    public:
        
        MutexRW(): fState(0), fWriteWaiters(0),fReadWaiters(0),fActiveReaders(0) {} ;

        void LockRead();
        void LockWrite();
        void Unlock();
        
        // Returns 0 on success, EBUSY on failure
        int TryLockWrite();
        int TryLockRead();
    
    private:
        enum {eMaxWait = 0x0FFFFFFF, eMultiThreadCondition = true, };
        enum {eActiveWriterState = -1, eNoWriterState = 0 };

        Mutex             fInternalLock;   // the internal lock         
        Cond              fReadersCond;    // the waiting readers             
        Cond              fWritersCond;    // the waiting writers             
        int                 fState;          // -1:writer,0:free,>0:readers 
        int                 fWriteWaiters;   // number of waiting writers   
        int                 fReadWaiters;    // number of waiting readers
        int                 fActiveReaders;  // number of active readers = fState >= 0;

        inline void AdjustState(int i) {  fState += i; };
        inline void AdjustWriteWaiters(int i) { fWriteWaiters += i; };
        inline void AdjustReadWaiters(int i) {  fReadWaiters += i; };
        inline void SetState(int i) { fState = i; };
        inline void SetWriteWaiters(int i) {  fWriteWaiters = i; };
        inline void SetReadWaiters(int i) { fReadWaiters = i; };
        
        inline void AddWriteWaiter() { AdjustWriteWaiters(1); };
        inline void RemoveWriteWaiter() {  AdjustWriteWaiters(-1); };
        
        inline void AddReadWaiter() { AdjustReadWaiters(1); };
        inline void RemoveReadWaiter() {  AdjustReadWaiters(-1); };
        
        inline void AddActiveReader() { AdjustState(1); };
        inline void RemoveActiveReader() {  AdjustState(-1); };
        
        
        inline bool WaitingWriters()  {return (bool) (fWriteWaiters > 0) ; }
        inline bool WaitingReaders()  {return (bool) (fReadWaiters > 0) ;}
        inline bool Active()          {return (bool) (fState != 0) ;}
        inline bool ActiveReaders()   {return (bool) (fState > 0) ;}
        inline bool ActiveWriter()    {return (bool) (fState < 0) ;} // only one
};

class   MutexReadWriteLocker
{
    public:
        MutexReadWriteLocker(MutexRW *inMutexPtr): fRWMutexPtr(inMutexPtr) {};
        ~MutexReadWriteLocker() { if (fRWMutexPtr != NULL) fRWMutexPtr->Unlock(); }


        void UnLock() { if (fRWMutexPtr != NULL) fRWMutexPtr->Unlock(); }
        void SetMutex(MutexRW *mutexPtr) {fRWMutexPtr = mutexPtr;}
        MutexRW*  fRWMutexPtr;
};
    
class   MutexReadLocker: public MutexReadWriteLocker
{
    public:

        MutexReadLocker(MutexRW *inMutexPtr) : MutexReadWriteLocker(inMutexPtr) 
        { if (MutexReadWriteLocker::fRWMutexPtr != NULL) MutexReadWriteLocker::fRWMutexPtr->LockRead(); }
            
        void Lock()         
        { if (MutexReadWriteLocker::fRWMutexPtr != NULL) MutexReadWriteLocker::fRWMutexPtr->LockRead(); }       
};

class   MutexWriteLocker: public MutexReadWriteLocker
{
    public:

        MutexWriteLocker(MutexRW *inMutexPtr) : MutexReadWriteLocker(inMutexPtr) 
        { if (MutexReadWriteLocker::fRWMutexPtr != NULL) MutexReadWriteLocker::fRWMutexPtr->LockWrite(); } 
                  
        void Lock()         
        { if (MutexReadWriteLocker::fRWMutexPtr != NULL) MutexReadWriteLocker::fRWMutexPtr->LockWrite(); }
};



#endif //_OSMUTEX_H_
