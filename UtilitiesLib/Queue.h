/*
    File:       Queue.h

    Contains:   implements Queue class           
*/

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "Headers.h"
#include "Cond.h"
#include "Mutex.h"
#include "Thread.h"
#include "MyAssert.h"

#define QUEUETESTING 0

class Queue;

class QueueElem 
{
    public:
        QueueElem(void* enclosingObject = NULL) : fNext(NULL), fPrev(NULL), fQueue(NULL),
                                                    fEnclosingObject(enclosingObject) {}
        virtual ~QueueElem() { /* Assert(fQueue == NULL); */ }

        bool IsMember(const Queue& queue) { return (&queue == fQueue); }
        bool IsMemberOfAnyQueue()     { return fQueue != NULL; }
        void* GetEnclosingObject()  { return fEnclosingObject; }
        void SetEnclosingObject(void* obj) { fEnclosingObject = obj; }

        QueueElem* Next() { return fNext; }
        QueueElem* Prev() { return fPrev; }
        Queue* InQueue()  { return fQueue; }
        inline void Remove();

    private:

        QueueElem*    fNext;
        QueueElem*    fPrev;
        Queue*        fQueue;
        void*         fEnclosingObject;

        friend class  Queue;
};


class Queue 
{
    public:
        Queue();
        ~Queue() {}

        void          	EnQueue(QueueElem* object);
        QueueElem*    	DeQueue();

        QueueElem*    	GetHead() { if (fLength > 0) return fSentinel.fPrev; return NULL; }
        QueueElem*    	GetTail() { if (fLength > 0) return fSentinel.fNext; return NULL; }
        UInt32        	GetLength() { return fLength; }
        
        void         	Remove(QueueElem* object);

#if QUEUETESTING
        static bool   	Test();
#endif

    protected:

        QueueElem     	fSentinel;
        UInt32        	fLength;
};


class QueueIter
{
    public:
        QueueIter(Queue* inQueue) : fQueueP(inQueue), fCurrentElemP(inQueue->GetHead()) {}
        QueueIter(Queue* inQueue, QueueElem* startElemP ) : fQueueP(inQueue)
        {
            if ( startElemP )
            {   
                Assert(startElemP->IsMember(*inQueue ));
                fCurrentElemP = startElemP;    
            }
            else
                fCurrentElemP = NULL;
        }
        ~QueueIter() {}
        
        void            Reset() { fCurrentElemP = fQueueP->GetHead(); }
        
        QueueElem*    GetCurrent() { return fCurrentElemP; }
        void          Next();
        
        bool          IsDone() { return fCurrentElemP == NULL; }
        
    private:
    
        Queue*        fQueueP;
        QueueElem*    fCurrentElemP;
};

class Queue_Blocking
{
    public:
        Queue_Blocking() {}
        ~Queue_Blocking() {}
        
        QueueElem*    DeQueueBlocking(Thread* inCurThread, SInt32 inTimeoutInMilSecs);
        QueueElem*    DeQueue(); //will not block
        void          EnQueue(QueueElem* obj);
        
        Cond*         GetCond()   { return &fCond; }
        Queue*        GetQueue()  { return &fQueue; }
        
    private:

        Cond           fCond;
        Mutex          fMutex;
        Queue          fQueue;
};

void QueueElem::Remove()
{
    if (fQueue != NULL)
        fQueue->Remove(this);
}

#endif //__QUEUE_H__
