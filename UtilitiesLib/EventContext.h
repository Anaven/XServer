#ifndef __EVENT_CONTEXT_H__
#define __EVENT_CONTEXT_H__



#include "Thread.h"
#include "Task.h"
#include "Ref.h"

#include "ev.h"

#define EVENTCONTEXT_DEBUG 0

class EventThread;

class EventContext
{
    public:
    
        // Constructor. Pass in the EventThread you would like to receive events
        EventContext(int inFileDesc, EventThread* inThread);
        virtual	~EventContext() { if (fAutoCleanup) this->Cleanup(); };

        //
        // InitNonBlocking
        //
        // Sets inFileDesc to be non-blocking. Once this is called, the
        // EventContext object "owns" the file descriptor, and will close it
        // when Cleanup is called. DON'T CALL CLOSE ON THE FD ONCE THIS IS CALLED!!!!
        void 	InitNonBlocking(int inFileDesc);

        // Cleanup. Will be called by the destructor, but can be called earlier
        void 	Cleanup();

        // Arms this EventContext. Pass in the events you would like to receive
        int 	RequestEvent(int theMask = EV_RE, int theMode = EPOLLET);
        void 	SetTask(Task* inTask)
        {  
            fTask = inTask; 
            if (EVENTCONTEXT_DEBUG)
            {
                if (fTask == NULL)  
                    printf("EventContext::SetTask context=%p task= NULL\n", (void *) this); 
                else 
                    printf("EventContext::SetTask context=%p task= %p name=%s\n",(void *) this,(void *) fTask, fTask->fTaskName); 
            }
        }

		// Don't cleanup this socket automatically
        void 	DontAutoCleanup() { fAutoCleanup = false; };
		
		// Direct access to the FD is not recommended, but is needed for modules
        // that want to use the Socket classes and need to request events on the fd.
        int 	GetSocketFD() { return fFileDesc; };
        
        enum 
		{
			kInvalidFileDesc = -1 
		};

    protected:

        // ProcessEvent
        // 
        // When an event occurs on this file descriptor, this function
        // will get called. Default behavior is to Signal the associated
        // task, but that behavior may be altered / overridden.
        // 
        // Currently, we always generate a Task::kReadEvent
    	virtual void ProcessEvent(int eventBits)
        {   
            if (EVENTCONTEXT_DEBUG)
            {
                if (fTask == NULL)  
                	printf("EventContext::ProcessEvent context=%p task=NULL\n",(void *) this); 
                else 
                    printf("EventContext::ProcessEvent context=%p task=%p TaskName=%s\n",(void *)this,(void *) fTask, fTask->fTaskName); 
            }
			
        	if (fTask != NULL)
        	{
                fTask->Signal(Task::kReadEvent); 
        	}
        }

        int fFileDesc;

	private:
        
        struct eventreq		fEventReq;

    	Ref           	fRef;
        UInt32	       	fUniqueID;
        StrPtrLen     	fUniqueIDStr;
        EventThread* 	fEventThread;
        bool           	fWatchEventCalled;
        int          	fEventBits;
        bool         	fAutoCleanup;

        Task*         	fTask;

    	static UInt32 	sUniqueID;
         
        friend class EventThread;
};

class EventThread : public Thread
{
    public:
    
        EventThread();
        virtual ~EventThread();

    	int WatchEvent(struct eventreq *theReq, int theMask, int theMode);
    	int RemoveEvent(int theFileDesc);

    	enum 
		{ 
			kDefaultEventSize = 2048,
		};
        
    private:
    
        virtual void Entry();
        
    	RefTable          	fRefTable;
        
        struct epoll_event*	fEvents;  
        int             	fEpfd;
        int             	fEventSize;
    	int	            	fListenfd;
        
        friend class EventContext;
};

#endif // __EVENT_CONTEXT_H__

