/*
    File:       ev.cpp

    Contains:   
    
*/

#define EV_DEBUGGING 0 //Enables a lot of printfs

#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>

#include "ev.h"
#include "OS.h"
#include "MyAssert.h"
#include "Thread.h"
#include "Mutex.h"

static int sEventSize = 512;
static int sEpfd = 0;
static struct epoll_event* sEvents;
static Mutex sMaxFDPosMutex;


static bool selecthasdata();
static int constructeventreq(struct eventreq* req, int fd, int event);


void epoll_startevents()
{
    sEpfd = ::epoll_create(sEventSize);
    Assert(sEpfd != -1);

    sEvents = new epoll_event[sEventSize];
}

int epoll_removeevent(int which)
{
    int theErr = ::epoll_ctl(sEpfd, EPOLL_CTL_DEL, which, NULL);

    return theErr;    
}

int epoll_watchevent(struct eventreq *req, int which)
{
    return select_modwatch(req, which);
}

int epoll_modwatch(struct eventreq *req, int which)
{
    {
        MutexLocker locker(&sMaxFDPosMutex);

        //Add or remove this fd from the specified sets
        if (which & EV_RE)
        {
    #if EV_DEBUGGING
            printf("modwatch: Enabling %d in readset\n", req->er_handle);
    #endif
            epoll_ctl(sEpfd, EPOLL_CTL_ADD, req->er_handle, &req->er_data);
        }
        else
        {
    #if EV_DEBUGGING
            printf("modwatch: Disbling %d in readset\n", req->er_handle);
    #endif
            epoll_ctl(sEpfd, EPOLL_CTL_DEL, req->er_handle, &req->er_data);
        }
        
        if (which & EV_WR)
        {
    #if EV_DEBUGGING
            printf("modwatch: Enabling %d in writeset\n", req->er_handle);
    #endif
            epoll_ctl(sEpfd, EPOLL_CTL_ADD, req->er_handle, &req->er_data);
        }
        else
        {
    #if EV_DEBUGGING
            printf("modwatch: Disabling %d in writeset\n", req->er_handle);
    #endif
            epoll_ctl(sEpfd, EPOLL_CTL_DEL, req->er_handle, &req->er_data);
        }
    }

    return 0;
}

int constructeventreq(struct eventreq* req, int fd, int event)
{   
    req->er_handle = fd;
    req->er_eventbits = event;

    return 0;
}

int select_waitevent(struct eventreq *req, void* /*onlyForMacOSX*/)
{
    //Check to see if we still have some select descriptors to process
    int theFDsProcessed = (int)sNumFDsProcessed;
    bool isSet = false;
    
    if (theFDsProcessed < sNumFDsBackFromSelect)
    {
        if (sInReadSet)
        {
            MutexLocker locker(&sMaxFDPosMutex);
#if EV_DEBUGGING
            qtss_printf("waitevent: Looping through readset starting at %d\n", sCurrentFDPos);
#endif
            while((!(isSet = FD_ISSET(sCurrentFDPos, &sReturnedReadSet))) && (sCurrentFDPos < sMaxFDPos)) 
                sCurrentFDPos++;        

            if (isSet)
            {   
#if EV_DEBUGGING
                qtss_printf("waitevent: Found an fd: %d in readset max=%d\n", sCurrentFDPos, sMaxFDPos);
#endif
                FD_CLR(sCurrentFDPos, &sReturnedReadSet);
                return constructeventreq(req, sCurrentFDPos, EV_RE);
            }
            else
            {
#if EV_DEBUGGING
                qtss_printf("waitevent: Stopping traverse of readset at %d\n", sCurrentFDPos);
#endif
                sInReadSet = false;
                sCurrentFDPos = 0;
            }
        }
        if (!sInReadSet)
        {
            MutexLocker locker(&sMaxFDPosMutex);
#if EV_DEBUGGING
            qtss_printf("waitevent: Looping through writeset starting at %d\n", sCurrentFDPos);
#endif
            while((!(isSet = FD_ISSET(sCurrentFDPos, &sReturnedWriteSet))) && (sCurrentFDPos < sMaxFDPos))
                sCurrentFDPos++;

            if (isSet)
            {
#if EV_DEBUGGING
                qtss_printf("waitevent: Found an fd: %d in writeset\n", sCurrentFDPos);
#endif
                FD_CLR(sCurrentFDPos, &sReturnedWriteSet);
                return constructeventreq(req, sCurrentFDPos, EV_WR);
            }
            else
            {
                // This can happen if another thread calls select_removeevent at just the right
                // time, setting sMaxFDPos lower than it was when select() was last called.
                // Becase sMaxFDPos is used as the place to stop iterating over the read & write
                // masks, setting it lower can cause file descriptors in the mask to get skipped.
                // If they are skipped, that's ok, because those file descriptors were removed
                // by select_removeevent anyway. We need to make sure to finish iterating over
                // the masks and call select again, which is why we set sNumFDsProcessed
                // artificially here.
                sNumFDsProcessed = sNumFDsBackFromSelect;
                Assert(sNumFDsBackFromSelect > 0);
            }
        }
    }
    
    if (sNumFDsProcessed > 0)
    {
        MutexLocker locker(&sMaxFDPosMutex);

#if EV_DEBUGGING
        qtss_printf("waitevent: Finished with all fds in set. Stopped traverse of writeset at %d maxFD = %d\n", sCurrentFDPos,sMaxFDPos);
#endif
        //We've just cycled through one select result. Re-init all the counting states
        sNumFDsProcessed = 0;
        sNumFDsBackFromSelect = 0;
        sCurrentFDPos = 0;
        sInReadSet = true;
    }
    
    
    
    while(!selecthasdata())
    {
        {
            MutexLocker locker(&sMaxFDPosMutex);
            //Prepare to call select. Preserve the read and write sets by copying their contents
            //into the corresponding "returned" versions, and then pass those into select
            ::::memcpy(&sReturnedReadSet, &sReadSet, sizeof(fd_set));
            ::memcpy(&sReturnedWriteSet, &sWriteSet, sizeof(fd_set));
        }

        SInt64  yieldDur = 0;
        SInt64  yieldStart;
        
        //Periodically time out the select call just in case we
        //are deaf for some reason
        // on platforw's where our threading is non-preemptive, just poll select

        struct timeval  tv;
        tv.tv_usec = 0;

    #if THREADING_IS_COOPERATIVE
        tv.tv_sec = 0;
        
        if ( yieldDur > 4 )
            tv.tv_usec = 0;
        else
            tv.tv_usec = 5000;
    #else
        tv.tv_sec = 15;
    #endif

#if EV_DEBUGGING
        qtss_printf("waitevent: about to call select\n");
#endif

        yieldStart = OS::Milliseconds();
        OSThread::ThreadYield();
        
        yieldDur = OS::Milliseconds() - yieldStart;
#if EV_DEBUGGING
        static SInt64   numZeroYields;
        
        if ( yieldDur > 1 )
        {
            qtss_printf( "select_waitevent time in OSThread::Yield() %i, numZeroYields %i\n", (SInt32)yieldDur, (SInt32)numZeroYields );
            numZeroYields = 0;
        }
        else
            numZeroYields++;

#endif

        sNumFDsBackFromSelect = ::select(sMaxFDPos+1, &sReturnedReadSet, &sReturnedWriteSet, NULL, &tv);

#if EV_DEBUGGING
        qtss_printf("waitevent: back from select. Result = %d\n", sNumFDsBackFromSelect);
#endif
    }
    

    if (sNumFDsBackFromSelect >= 0)
        return EINTR;   //either we've timed out or gotten some events. Either way, force caller
                        //to call waitevent again.
    return sNumFDsBackFromSelect;
}

bool selecthasdata()
{
    if (sNumFDsBackFromSelect < 0)
    {
        int err=OSThread::GetErrno();
        
#if EV_DEBUGGING
        if (err == ENOENT) 
        {
             qtss_printf("selectHasdata: found error ENOENT==2 \n");
        }
#endif

        if ( 
#if __solaris__
            err == ENOENT || // this happens on Solaris when an HTTP fd is closed
#endif      
            err == EBADF || //this might happen if a fd is closed right before calling select
            err == EINTR 
           ) // this might happen if select gets interrupted
             return false;
        return true;//if there is an error from select, we want to make sure and return to the caller
    }
        
    if (sNumFDsBackFromSelect == 0)
        return false;//if select returns 0, we've simply timed out, so recall select
    
    if (FD_ISSET(sPipes[0], &sReturnedReadSet))
    {
#if EV_DEBUGGING
        qtss_printf("selecthasdata: Got some data on the pipe fd\n");
#endif
        //we've gotten data on the pipe file descriptor. Clear the data.
        // increasing the select buffer fixes a hanging problem when the Darwin server is under heavy load
        // CISCO contribution
        char theBuffer[4096]; 
        (void)::read(sPipes[0], &theBuffer[0], 4096);

        FD_CLR(sPipes[0], &sReturnedReadSet);
        sNumFDsBackFromSelect--;
        
        {
            //Check the fds to close array, and if there are any in it, close those descriptors
            MutexLocker locker(&sMaxFDPosMutex);
            for (UInt32 theIndex = 0; ((sFDsToCloseArray[theIndex] != -1) && (theIndex < sizeof(fd_set) * 8)); theIndex++)
            {
                (void)::close(sFDsToCloseArray[theIndex]);
                sFDsToCloseArray[theIndex] = -1;
            }
        }
    }
    Assert(!FD_ISSET(sPipes[0], &sReturnedWriteSet));
    
    if (sNumFDsBackFromSelect == 0)
        return false;//if the pipe file descriptor is the ONLY data we've gotten, recall select
    else
        return true;//we've gotten a real event, return that to the caller
}

#endif //!MACOSXEVENTQUEUE

