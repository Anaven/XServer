/*
    File:       TimeoutTask.h

    Contains:   Just like a normal task, but can be scheduled for timeouts. Unlike
                IdleTask, which is VERY aggressive about being on time, but high
                overhead for maintaining the timing information, this is a low overhead,
                low priority timing mechanism. Timeouts may not happen exactly when
                they are supposed to, but who cares?
            
*/

#ifndef __TIMEOUTTASK_H__
#define __TIMEOUTTASK_H__


#include "StrPtrLen.h"
#include "IdleTask.h"

#include "Thread.h"
#include "Queue.h"
#include "Mutex.h"
#include "OS.h"

#define TIMEOUT_DEBUGGING 0 //messages to help debugging timeouts

class TimeoutTaskThread : public IdleTask
{
    public:
    
        //All timeout tasks get timed out from this thread
                    TimeoutTaskThread() : IdleTask(), fMutex() { this->SetTaskName("TimeoutTask"); }
        virtual     ~TimeoutTaskThread(){}

    private:
        
        //this thread runs every minute and checks for timeouts
        enum
        {
            kIntervalSeconds = 60   //UInt32
        };

        virtual SInt64 Run();
        
        Mutex	fMutex;
        Queue	fQueue;
        
        friend class TimeoutTask;
};

class TimeoutTask
{
    //TimeoutTask is not a derived object off of Task, to add flexibility as
    //to how this object can be utilitized
    
    public:
    
        //Call Initialize before using this class
        static  void Initialize();
        //Pass in the task you'd like to send timeouts to. 
        //Also pass in the timeout you'd like to use. By default, the timeout is 0 (NEVER).
        TimeoutTask(Task* inTask, SInt64 inTimeoutInMilSecs = 60);
        ~TimeoutTask();
        
        //MODIFIERS

        // Changes the timeout time, also refreshes the timeout
        void        SetTimeout(SInt64 inTimeoutInMilSecs);
        
        // Specified task will get a Task::kTimeoutEvent if this
        // function isn't called within the timeout period
        void        RefreshTimeout() { fTimeoutAtThisTime = OS::Milliseconds() + fTimeoutInMilSecs; Assert(fTimeoutAtThisTime > 0); }
        
        void        SetTask(Task* inTask) { fTask = inTask; }
    private:
    
        Task*   	fTask;
        SInt64   	fTimeoutAtThisTime;
        SInt64    	fTimeoutInMilSecs;
        //for putting on our global queue of timeout tasks
        QueueElem 	fQueueElem;
        
        static TimeoutTaskThread*   sThread;
        
        friend class TimeoutTaskThread;
};
#endif //__TIMEOUTTASK_H__

