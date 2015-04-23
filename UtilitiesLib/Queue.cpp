/*
    File:       Queue.cpp

    Contains:   implements Queue class    
*/

#include "Queue.h"


Queue::Queue() : fLength(0)
{
    fSentinel.fNext = &fSentinel;
    fSentinel.fPrev = &fSentinel;
}


void Queue::EnQueue(QueueElem* elem)
{
    Assert(elem != NULL);  
    if (elem->fQueue == this)
        return;

    Assert(elem->fQueue == NULL);    /// A QueueElem can't in two queues at the same time
    elem->fNext = fSentinel.fNext;
    elem->fPrev = &fSentinel;
    elem->fQueue = this;
    fSentinel.fNext->fPrev = elem;
    fSentinel.fNext = elem;
    fLength++;
}


QueueElem* Queue::DeQueue()
{
    if (fLength > 0)
    {
        QueueElem* elem = fSentinel.fPrev;
        Assert(fSentinel.fPrev != &fSentinel);
        elem->fPrev->fNext = &fSentinel;
        fSentinel.fPrev = elem->fPrev;
        elem->fQueue = NULL;
        fLength--;
        return elem;
    }
    else
        return NULL;
}


void Queue::Remove(QueueElem* elem)
{
    Assert(elem != NULL);
    Assert(elem != &fSentinel);
    
    if (elem->fQueue == this)
    {
        elem->fNext->fPrev = elem->fPrev;
        elem->fPrev->fNext = elem->fNext;
        elem->fQueue = NULL;
        fLength--;
    }
}

#if QUEUETESTING
bool Queue::Test()
{
    Queue theVictim;
    void *x = (void*)1;
    QueueElem theElem1(x);
    x = (void*)2;
    QueueElem theElem2(x);
    x = (void*)3;
    QueueElem theElem3(x);
    
    if (theVictim.GetHead() != NULL)
        return false;
    if (theVictim.GetTail() != NULL)
        return false;
    
    theVictim.EnQueue(&theElem1);
    if (theVictim.GetHead() != &theElem1)
        return false;
    if (theVictim.GetTail() != &theElem1)
        return false;
    
    QueueElem* theElem = theVictim.DeQueue();
    if (theElem != &theElem1)
        return false;
    
    if (theVictim.GetHead() != NULL)
        return false;
    if (theVictim.GetTail() != NULL)
        return false;
    
    theVictim.EnQueue(&theElem1);
    theVictim.EnQueue(&theElem2);

    if (theVictim.GetHead() != &theElem1)
        return false;
    if (theVictim.GetTail() != &theElem2)
        return false;
        
    theElem = theVictim.DeQueue();
    if (theElem != &theElem1)
        return false;

    if (theVictim.GetHead() != &theElem2)
        return false;
    if (theVictim.GetTail() != &theElem2)
        return false;

    theElem = theVictim.DeQueue();
    if (theElem != &theElem2)
        return false;

    theVictim.EnQueue(&theElem1);
    theVictim.EnQueue(&theElem2);
    theVictim.EnQueue(&theElem3);

    if (theVictim.GetHead() != &theElem1)
        return false;
    if (theVictim.GetTail() != &theElem3)
        return false;

    theElem = theVictim.DeQueue();
    if (theElem != &theElem1)
        return false;

    if (theVictim.GetHead() != &theElem2)
        return false;
    if (theVictim.GetTail() != &theElem3)
        return false;

    theElem = theVictim.DeQueue();
    if (theElem != &theElem2)
        return false;

    if (theVictim.GetHead() != &theElem3)
        return false;
    if (theVictim.GetTail() != &theElem3)
        return false;

    theElem = theVictim.DeQueue();
    if (theElem != &theElem3)
        return false;

    theVictim.EnQueue(&theElem1);
    theVictim.EnQueue(&theElem2);
    theVictim.EnQueue(&theElem3);
    
    QueueIter theIterVictim(&theVictim);
    if (theIterVictim.IsDone())
        return false;
    if (theIterVictim.GetCurrent() != &theElem3)
        return false;
    theIterVictim.Next();
    if (theIterVictim.IsDone())
        return false;
    if (theIterVictim.GetCurrent() != &theElem2)
        return false;
    theIterVictim.Next();
    if (theIterVictim.IsDone())
        return false;
    if (theIterVictim.GetCurrent() != &theElem1)
        return false;
    theIterVictim.Next();
    if (!theIterVictim.IsDone())
        return false;
    if (theIterVictim.GetCurrent() != NULL)
        return false;

    theVictim.Remove(&theElem1);

    if (theVictim.GetHead() != &theElem2)
        return false;
    if (theVictim.GetTail() != &theElem3)
        return false;

    theVictim.Remove(&theElem1);

    if (theVictim.GetHead() != &theElem2)
        return false;
    if (theVictim.GetTail() != &theElem3)
        return false;

    theVictim.Remove(&theElem3);

    if (theVictim.GetHead() != &theElem2)
        return false;
    if (theVictim.GetTail() != &theElem2)
        return false;

    return true;
}   
#endif


void QueueIter::Next()
{
    if (fCurrentElemP == fQueueP->GetTail())
        fCurrentElemP = NULL;
    else
        fCurrentElemP = fCurrentElemP->Prev();
}


QueueElem* Queue_Blocking::DeQueueBlocking(Thread* inCurThread, SInt32 inTimeoutInMilSecs)
{
    MutexLocker theLocker(&fMutex);

    if (fQueue.GetLength() == 0) 
        fCond.Wait(&fMutex, inTimeoutInMilSecs);

    QueueElem* retval = fQueue.DeQueue();
    return retval;
}

QueueElem* Queue_Blocking::DeQueue()
{
    MutexLocker theLocker(&fMutex);

    QueueElem* retval = fQueue.DeQueue(); 
    return retval;
}


void Queue_Blocking::EnQueue(QueueElem* obj)
{
    {
        MutexLocker theLocker(&fMutex);
        fQueue.EnQueue(obj);
    }
    fCond.Signal();
}
