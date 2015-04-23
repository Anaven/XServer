/*
    File:       Thread.h

    Contains:   A thread abstraction
*/

// Thread.h
#ifndef __BASE_THREAD_H__
#define __BASE_THREAD_H__


#include <errno.h>
#include <pthread.h>

#include "Headers.h"

class Thread
{

public:
    
	static void     	Initialize();
    
                        Thread();
    virtual             ~Thread();
    
    //
    // Derived classes must implement their own entry function
    virtual void        Entry() = 0;
    void                Start();
                
    static void         ThreadYield();
    static void         Sleep(UInt32 inMsec);

	void				SetAffinity(UInt32 theMask = 1);
    void                Join();
    void                SendStopRequest() { fStopRequested = true; }
    bool                IsStopRequested() { return fStopRequested; }
    void                StopAndWaitForThread();

    void*               GetThreadData()         { return fThreadData; }
    void                SetThreadData(void* inThreadData) { fThreadData = inThreadData; }

    static int          GetErrno() { return errno; }
    static pthread_t    GetCurrentThreadID() { return ::pthread_self(); }

	static	Thread*    	GetCurrent();
    
private:

	static pthread_key_t    gMainKey;

    bool                    fStopRequested;
    bool                    fJoined;

    pthread_t               fThreadID;
    void*                   fThreadData;

    static void*            _Entry(void* inThread);
};

#endif

