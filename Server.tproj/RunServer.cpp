/*
    File:       main.cpp

    Contains:   main function to drive access server.

    

*/

#include <errno.h>

#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <sys/stat.h>

#include "RunServer.h"
#include "OS.h"
#include "Memory.h"
#include "Thread.h"
#include "Socket.h"
#include "SocketUtils.h"
#include "Task.h"
#include "IdleTask.h"
#include "TimeoutTask.h"

XServer* sServer = NULL;
bool sHasPID = false;

int StartServer(ConfigPrefsParser *inConfigPrefsParser, UInt16 inPort, UInt32 inInitialState, bool inDontFork)
{
    bool doneStartingUp = false;
    int theServerState = XServer::kStartingUpState;

    //Initialize utility classes
    OS::Initialize();
    Thread::Initialize();
    
    Socket::Initialize();
    SocketUtils::Initialize(!inDontFork);

    sServer = new XServer();

    bool createListeners = true;
    if (XServer::kShuttingDownState == inInitialState) 
        createListeners = false;

    sServer->Initialize(inConfigPrefsParser, inPort, createListeners);

    if (inInitialState == XServer::kShuttingDownState) 
        return inInitialState; 

    if (sServer->GetServerState() != XServer::kFatalErrorState)
    {
        UInt32 numShortTaskThreads = 0;
        UInt32 numBlockingThreads = 0;
        UInt32 numThreads = 0;
        UInt32 numProcessors = 0;

        numShortTaskThreads = sServer->GetNumShortThreads();
        if (numShortTaskThreads == 0) 
        {
            numProcessors = OS::GetNumProcessors();
            // 1 worker thread per processor, up to 2 threads.
            if (numProcessors > 2)
                numShortTaskThreads = 2;
            else
                numShortTaskThreads = numProcessors;
        }

        numBlockingThreads = sServer->GetNumBlockingThreads();
        if (numBlockingThreads == 0)
            numBlockingThreads = 1;
            
        if (numShortTaskThreads == 0)
            numShortTaskThreads = 1;
        
        numThreads = numShortTaskThreads + numBlockingThreads;
        TaskThreadPool::SetNumShortTaskThreads(numShortTaskThreads);
        TaskThreadPool::SetNumBlockingTaskThreads(numBlockingThreads);
        TaskThreadPool::AddThreads(numThreads, numProcessors);
        sServer->InitNumThreads(numThreads);
        
        TimeoutTask::Initialize();      // The TimeoutTask mechanism is task based,
                                        // we therefore must do this after adding task threads
                                        // this be done before starting the sockets and server tasks
    }

    //Make sure to do this stuff last. Because these are all the threads that
    //do work in the server, this ensures that no work can go on while the server
    //is in the process of staring up
    if (sServer->GetServerState() != XServer::kFatalErrorState)
    {
        IdleTask::Initialize();
        Socket::StartThread();
        Thread::Sleep(1000);

        sServer->StartTasks();
        theServerState = sServer->GetServerState();
    }

    if (theServerState != XServer::kFatalErrorState)
    {
        CleanPid(true);
        WritePid(!inDontFork);

        doneStartingUp = true;
        printf("XServer done starting up\n");
    }

    // SWITCH TO RUN USER AND GROUP ID
     if (!sServer->SwitchPersonality())
         theServerState = XServer::kFatalErrorState;

    // Tell the caller whether the server started up or not
    return theServerState;
}

void WritePid(bool forked)
{
    // WRITE PID TO FILE
    char* thePidFileName = sServer->GetPidFilePath();
    FILE *thePidFile = fopen(thePidFileName, "w");
    if(thePidFile)
    {
        if (!forked)
            fprintf(thePidFile,"%d\n",getpid());    // write own pid
        else
        {
            fprintf(thePidFile,"%d\n",getppid());    // write parent pid
            fprintf(thePidFile,"%d\n",getpid());    // and our own pid in the next line
        }                
        fclose(thePidFile);
        sHasPID = true;
    }
}

void CleanPid(bool force)
{
    if (sHasPID || force)
    {
        char* thePidFileName = sServer->GetPidFilePath();
        unlink(thePidFileName);
    }
}

void RunServer()
{   
    bool restartServer = false;
    
    //just wait until someone stops the server or a fatal error occurs.
    
    int theServerState = sServer->GetServerState();
    while ((theServerState != XServer::kShuttingDownState) && (theServerState != XServer::kFatalErrorState))
    {
        #if MEMORY_DEBUGGING
		/*
    	printf("==================================================\n");
    	printf("MessageSession Number %d\n", sServer->GetNumMessageSession());
    	printf("AudioSession Number %d\n", sServer->GetNumAudioSession());
    	printf("NetStream Number %d\n", sServer->GetNumVideoSession());
    	printf("\n");

        {
        	MutexLocker theLocker(Memory::GetTagQueueMutex());
        	Queue* theTagQueue = Memory::GetTagQueue();

        	for(QueueIter theIter(theTagQueue); !theIter.IsDone(); theIter.Next())
            {
            	Memory::TagElem* elem = (Memory::TagElem*)theIter.GetCurrent()->GetEnclosingObject();
            	printf("Not Free %s:%d, Ref Num %d, Total %d Byte\n",
                	elem->fileName, elem->line, elem->numObjects, elem->totMemory);
            }
        }
        
    	printf("==================================================\n");    
		*/
        #endif

        Thread::Sleep(2 * 60 * 1000);
        theServerState = sServer->GetServerState();
        if (theServerState == XServer::kIdleState)
            sServer->KillAllSessions();

        // theServerState = XServer::kShuttingDownState;
    }
    
    //
    // Kill all the sessions and wait for them to die,
    // but don't wait more than 5 seconds
    sServer->KillAllSessions();
    //Now, make sure that the server can't do any work
    TaskThreadPool::RemoveThreads();
    
    //now that the server is definitely stopped, it is safe to initate
    //the shutdown process
    
    delete sServer;
    
    //ok, we're ready to exit. If we're quitting because of some fatal error
    //while running the server, make sure to let the parent process know by
    //exiting with a nonzero status. Otherwise, exit with a 0 status
    if (theServerState == XServer::kFatalErrorState || restartServer)
        ::exit (-2); //-2 signals parent process to restart server
}
