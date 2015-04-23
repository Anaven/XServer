#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>

#include "atomic.h"

#include "OS.h"
#include "MyAssert.h"
#include "EventContext.h"
#include "Thread.h"
#include "Memory.h"

//unsigned int EventContext::sUniqueID = 1;

#define EVENT_CONTEXT_DEBUG 0

#if EVENT_CONTEXT_DEBUG
#include "OS.h"
#endif

UInt32 EventContext::sUniqueID = 1;

EventContext::EventContext(int inFileDesc, EventThread* inThread)
 :  fFileDesc(inFileDesc), 
    fUniqueID(0),
    fUniqueIDStr((char*)&fUniqueID, sizeof(fUniqueID)),
    fEventThread(inThread),
    fWatchEventCalled(false),
    fAutoCleanup(true)
{}


void EventContext::InitNonBlocking(int inFileDesc)
{
    fFileDesc = inFileDesc;
     
    int flag = ::fcntl(fFileDesc, F_GETFL, 0);
    int err =::fcntl(fFileDesc, F_SETFL, flag | O_NONBLOCK); 

    AssertV(err == 0, Thread::GetErrno());
}


void EventContext::Cleanup()
{
    int err = 0;

    if (fFileDesc != kInvalidFileDesc)
    {
        //if this object is registered in the table, unregister it now
        if (fUniqueID > 0)
        {
            fEventThread->fRefTable.UnRegister(&fRef);
            fEventThread->RemoveEvent(fFileDesc);
        }
        
        err = ::close(fFileDesc);
    }

    fFileDesc = kInvalidFileDesc;    
    fUniqueID = 0;

    AssertV(err == 0, Thread::GetErrno());
}


int EventContext::RequestEvent(int theMask, int theMode)
{
    if(!fWatchEventCalled)
    {   
        //allocate a Unique ID for this socket, and add it to the ref table
        
        //fUniqueID = (UInt32)this;
        if (!compare_and_store(10000000, 1, &sUniqueID))
            fUniqueID = (UInt32)atomic_add(&sUniqueID, 1);
        else
            fUniqueID = 1;

        fRef.Set(fUniqueIDStr, this);
        int err = fEventThread->fRefTable.Register(&fRef);
        Assert(err == 0);

        //fill out the eventreq data structure
        memset(&fEventReq, 0, sizeof(fEventReq));
        fEventReq.er_type       = EV_FD;
        fEventReq.er_handle     = fFileDesc;
        fEventReq.er_eventbits  = theMask;
        fEventReq.er_data       = (void*)fUniqueID;

        fWatchEventCalled = true;

        if (fEventThread->WatchEvent(&fEventReq, theMask, theMode) != 0)
            AssertV(false, Thread::GetErrno());
    }

    return 0;
}


EventThread::EventThread()
 :  fEvents(NULL),
    fEventSize(kDefaultEventSize)
{
    fEpfd = epoll_create(fEventSize);
    Assert(fEpfd != -1);

    fEvents = new epoll_event[fEventSize];
}


EventThread::~EventThread()
{
    delete []fEvents;
}


int EventThread::WatchEvent(struct eventreq *theReq, int theMask, int theMode)
{
    struct epoll_event theEvent;

    memset(&theEvent, 0, sizeof(struct epoll_event));
    theEvent.data.ptr = theReq;
    theEvent.events = theEvent.events | theMode;
    
    if(theMask & EV_RE)
    {    
#if EVENT_CONTEXT_DEBUG
        printf("WatchEvent: Enabling %d in readset\n", theReq->er_handle);
#endif
        theEvent.events = theEvent.events | EPOLLIN;
    }
    
    if(theMask & EV_WR)
    {
#if EVENT_CONTEXT_DEBUG
        printf("WatchEvent: Enabling %d in writeset\n", theReq->er_handle);
#endif    
        theEvent.events = theEvent.events | EPOLLOUT;
    }

/* 
    if (theReq->er_handle > sMaxFDPos)
        sMaxFDPos = theReq->er_handle;

#if EVENT_CONTEXT_DEBUG
    printf("WatchEvent: MaxFDPos=%d\n", sMaxFDPos);
#endif
*/
    int theErr = ::epoll_ctl(fEpfd, EPOLL_CTL_ADD, theReq->er_handle, &theEvent);
    
    return theErr;
}


int EventThread::RemoveEvent(int theFileDesc)
{
    int theErr = ::epoll_ctl(fEpfd, EPOLL_CTL_DEL, theFileDesc, NULL);

    return theErr;
}

void EventThread::Entry()
{    
    struct eventreq *theCurrentEvent = NULL;
    EventContext* theContext = NULL;
    int nfds = 0;

    while(true)
    {
        int theErrno = EINTR;
        while (theErrno == EINTR)
        {
            nfds = epoll_wait(fEpfd, fEvents, fEventSize, kDefaultEventSize);
            if (nfds >= 0)
                theErrno = nfds;
            else
                theErrno = Thread::GetErrno();
        }

        AssertV(theErrno >= 0, theErrno);

        for (int i = 0; i < nfds; i++)
        {
            theCurrentEvent = (struct eventreq *)fEvents[i].data.ptr;

            if (theCurrentEvent->er_data != NULL)
            {
                //The cookie in this event is an ObjectID. Resolve that objectID into
                //a pointer.
                StrPtrLen idStr((char*)&theCurrentEvent->er_data, sizeof(theCurrentEvent->er_data));
                Ref* ref = fRefTable.Resolve(&idStr);
                if (ref != NULL)
                {
                    theContext = (EventContext*)ref->GetObject();
                    
                    theContext->ProcessEvent(theCurrentEvent->er_eventbits);
                    fRefTable.Release(ref);
                }
            }
        }
        
#if EVENT_CONTEXT_DEBUG
        SInt64  yieldStart = OS::Milliseconds();
#endif

        this->ThreadYield();

#if EVENT_CONTEXT_DEBUG
        SInt64	yieldDur = OS::Milliseconds() - yieldStart;
        static SInt64	numZeroYields;

        if ( yieldDur > 1 )
        {
            printf( "EventThread time in THread::Yield %i, numZeroYields %i\n", (SInt32)yieldDur, (SInt32)numZeroYields );
            numZeroYields = 0;
        }
        else
            numZeroYields++;
#endif
    }
}

