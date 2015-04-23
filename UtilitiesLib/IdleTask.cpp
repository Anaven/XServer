/*
    File:       IdleTask.cpp

    Contains:   IdleTasks are identical to normal tasks (see task.h) with one exception:
    
                You can schedule them for timeouts. If you call SetIdleTimer
                on one, after the time has elapsed the task object will receive an
                kIdleEvent event. 
                
*/

#include "Memory.h"
#include "IdleTask.h"
#include "OS.h"

//IDLETASKTHREAD IMPLEMENTATION:
IdleTaskThread*     IdleTask::sIdleThread = NULL;

void IdleTaskThread::SetIdleTimer(IdleTask *activeObj, SInt64 msec)
{
    //note: Heap doesn't support a random remove, so this function
    //won't change the timeout value if there is already one set
    if (activeObj->fIdleElem.IsMemberOfAnyHeap())
        return;
    activeObj->fIdleElem.SetValue(OS::Milliseconds() + msec);
    
    {
        MutexLocker locker(&fHeapMutex);
        fIdleHeap.Insert(&activeObj->fIdleElem);
    }
    fHeapCond.Signal();
}

void IdleTaskThread::CancelTimeout(IdleTask* idleObj)
{
    Assert(idleObj != NULL);
    MutexLocker locker(&fHeapMutex);
    fIdleHeap.Remove(&idleObj->fIdleElem);  
}

void IdleTaskThread::Entry()
{
    MutexLocker locker(&fHeapMutex);
    
    while (true)
    {
        //if there are no events to process, block.
        if (fIdleHeap.CurrentHeapSize() == 0)
            fHeapCond.Wait(&fHeapMutex);
        SInt64 msec = OS::Microseconds();
        
        //pop elements out of the heap as long as their timeout time has arrived
        while ((fIdleHeap.CurrentHeapSize() > 0) && (fIdleHeap.PeekMin()->GetValue() <= msec))
        {
            IdleTask* elem = (IdleTask*)fIdleHeap.ExtractMin()->GetEnclosingObject();
            Assert(elem != NULL);

            elem->Signal(Task::kIdleEvent);
        }
                        
        //we are done sending idle events. If there is a lowest tick count, then
        //we need to sleep until that time.
        if (fIdleHeap.CurrentHeapSize() > 0)
        {
            SInt64 timeoutTime = fIdleHeap.PeekMin()->GetValue();
            //because sleep takes a 32 bit number
            timeoutTime -= msec;
            Assert(timeoutTime > 0);
            UInt32 smallTime = (UInt32)timeoutTime;
            fHeapCond.Wait(&fHeapMutex, smallTime);
        }
    }   
}

void IdleTask::Initialize()
{
    if (sIdleThread == NULL)
    {
        sIdleThread = NEW IdleTaskThread();
        sIdleThread->Start();
		UInt32 theCPUs = OS::GetNumProcessors();
		sIdleThread->SetAffinity(theCPUs - 1); 
    }
}

IdleTask::~IdleTask()
{
    //clean up stuff used by idle thread routines
    Assert(sIdleThread != NULL);
    
    MutexLocker locker(&sIdleThread->fHeapMutex);

    //Check to see if there is a pending timeout. If so, get this object
    //out of the heap
    if (fIdleElem.IsMemberOfAnyHeap())
        sIdleThread->CancelTimeout(this);
}

