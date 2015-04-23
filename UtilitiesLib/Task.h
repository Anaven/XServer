/*
    File:       Task.h

    Contains:   Tasks are objects that can be scheduled. To schedule a task, you call its
                signal method, and pass in an event (events are bits and all events are defined
                below).
                
                Once Signal() is called, the task object will be scheduled. When it runs, its
                Run() function will get called. In order to clear the event, the derived task
                object must call GetEvents() (which returns the events that were sent).
                
                Calling GetEvents() implicitly "clears" the events returned. All events must
                be cleared before the Run() function returns, or Run() will be invoked again
                immediately.
*/

#ifndef __TASK_H__
#define __TASK_H__

#include <string.h>

#include "Headers.h"
#include "Queue.h"
#include "Heap.h"
#include "Thread.h"
#include "MutexRW.h"

#define TASK_DEBUG 0
#define TASK_THREAD_DEBUG 0

class TaskThread;

class Task
{
    public:
        
        typedef unsigned int EventFlags;

        //EVENTS
        //here are all the events that can be sent to a task
        enum
        {
            kKillEvent        	=	0x1 << 0x0, //these are all of type "EventFlags"
            kIdleEvent     		=	0x1 << 0x1,
            kStartEvent     	=	0x1 << 0x2,
            kTimeoutEvent     	= 	0x1 << 0x3,
       
            //socket events
            kReadEvent        	=  	0x1 << 0x4, //All of type "EventFlags"
            kWriteEvent      	=  	0x1 << 0x5,
           
            //update event
            kUpdateEvent       	=  	0x1 << 0x6,

        	kExpireEvent		=	0x1 << 0x7,
        };
        
        //CONSTRUCTOR / DESTRUCTOR
        //You must assign priority at create time.
        Task();
        virtual	~Task() {}

        //return:
        // >0-> invoke me after this number of MilSecs with a kIdleEvent
        // 0 don't reinvoke me at all.
        //-1 delete me
        //Suggested practice is that any task should be deleted by returning true from the
        //Run function. That way, we know that the Task is not running at the time it is
        //deleted. This object provides no protection against calling a method, such as Signal,
        //at the same time the object is being deleted (because it can't really), so watch
        //those dangling references!
        virtual SInt64	Run() = 0;
        
        //Send an event to this task.
        void	Signal(EventFlags eventFlags);
        void	GlobalUnlock();     
        bool	Valid(); // for debugging
    	char	fTaskName[48];
    	void	SetTaskName(const char* name);
        
        void	SetDefaultThread(TaskThread* defaultThread) { fDefaultThread = defaultThread; }
        void	SetThreadPicker(unsigned int* picker);
        static unsigned int* GetBlockingTaskThreadPicker() {return &sBlockingTaskThreadPicker; }
        
    protected:
    
        //Only the tasks themselves may find out what events they have received
        EventFlags	GetEvents();
        
        // ForceSameThread
        //
        // A task, inside its run function, may want to ensure that the same task thread
        // is used for subsequent calls to Run(). This may be the case if the task is holding
        // a mutex between calls to run. By calling this function, the task ensures that the
        // same task thread will be used for the next call to Run(). It only applies to the
        // next call to run.
        void	ForceSameThread()   
        {
            fUseThisThread = (TaskThread *)Thread::GetCurrent();
            Assert(fUseThisThread != NULL);
            if (TASK_DEBUG) 
            {
            	if (fTaskName[0] == 0) 
                    ::strcpy(fTaskName, " corrupt task");
            	printf("Task::ForceSameThread fUseThisThread %p task %s enque elem=%p enclosing %p\n", (void*) fUseThisThread, fTaskName,(void *)&fTaskQueueElem, (void *)this);
            }
        }
		
        SInt64	CallLocked()       
        {   
        	ForceSameThread();
            fWriteLock = true;
           	return (SInt64) 10; // minimum of 10 milliseconds between locks
        }

    private:

        enum
        {
            kAlive =            0x80000000, //EventFlags, again
            kAliveOff =         0x7fffffff
        };

        void            SetTaskThread(TaskThread *thread);
        
        EventFlags      fEvents;
        TaskThread*     fUseThisThread;
        TaskThread*     fDefaultThread;
        bool          	fWriteLock;

        //This could later be optimized by using a timing wheel instead of a heap,
        //and that way we wouldn't need both a heap elem and a queue elem here (just queue elem)
        HeapElem      fTimerHeapElem;
        QueueElem     fTaskQueueElem;
        
        unsigned int *pickerToUse;
        //Variable used for assigning tasks to threads in a round-robin fashion
        static unsigned int sShortTaskThreadPicker; //default picker
        static unsigned int sBlockingTaskThreadPicker;
        
        friend class	TaskThread; 
};

class TaskThread : public Thread
{
    public:
    
        //Implementation detail: all tasks get run on TaskThreads.
        
        TaskThread() :	Thread(), fTaskThreadPoolElem() { fTaskThreadPoolElem.SetEnclosingObject(this); }
    	virtual ~TaskThread() { this->StopAndWaitForThread(); }
           
    private:
    
        enum { kMinWaitTimeInMilSecs = 10 };

        virtual void    Entry();
        Task*           WaitForTask();
        
        QueueElem     fTaskThreadPoolElem;
        
        Heap              fHeap;
        Queue_Blocking    fTaskQueue;
        
        friend class Task;
        friend class TaskThreadPool;
};

//Because task threads share a global queue of tasks to execute,
//there can only be one pool of task threads. That is why this object
//is static.
class TaskThreadPool {
public:

    //Adds some threads to the pool
    static bool	AddThreads(UInt32 numToAdd, UInt32 totalCPUNum = 1); // creates the threads: takes NumShortTaskThreads + NumBLockingThreads,  sets num short task threads.
    static void	SwitchPersonality( char *user = NULL, char *group = NULL);
    static void	RemoveThreads();
    static TaskThread* GetThread(UInt32 index);
    static SInt32  GetNumThreads() { return sNumTaskThreads; }
    static void SetNumShortTaskThreads(UInt32 numToAdd) { sNumShortTaskThreads = numToAdd; }
    static void SetNumBlockingTaskThreads(UInt32 numToAdd) { sNumBlockingTaskThreads = numToAdd; }
    
private:

    static TaskThread**	sTaskThreadArray;
    static UInt32		sNumTaskThreads;
    static UInt32		sNumShortTaskThreads;
    static UInt32		sNumBlockingTaskThreads;
    
    static MutexRW		sMutexRW;
    
    friend class Task;
    friend class TaskThread;
};

#endif
