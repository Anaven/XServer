/*
    File:       BufferPool.h

    Contains:   Fast access to fixed size buffers.
    
    Written By: Denis Serenyi
    
*/

#ifndef __BUFFER_POOL_H__
#define __BUFFER_POOL_H__

#include "Queue.h"
#include "Mutex.h"

class BufferPool
{
    public:
    
        BufferPool(UInt32 inBufferSize) : fBufSize(inBufferSize), fTotNumBuffers(0) {}
        
        //
        // This object currently *does not* clean up for itself when
        // you destruct it!
        ~BufferPool() {}

		UInt32 GetBufSize() { return fBufSize; }
        //
        // ACCESSORS
        UInt32  GetTotalNumBuffers() { return fTotNumBuffers; }
        UInt32  GetNumAvailableBuffers() { return fQueue.GetLength(); }
        
        //
        // All these functions are thread-safe
        
        //
        // Gets a buffer out of the pool. This buffer must be replaced
        // by calling Put when you are done with it.
        void*   Get();
        
        //
        // Returns a buffer retreived by Get back to the pool.
        void    Put(void* inBuffer);
    
    private:
    
        Mutex   fMutex;
        Queue   fQueue;
        UInt32  fBufSize;
        UInt32  fTotNumBuffers;
};

#endif //__BUFFER_POOL_H__
