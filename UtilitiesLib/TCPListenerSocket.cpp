#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "Task.h"
#include "TCPListenerSocket.h"


int TCPListenerSocket::Listen(UInt32 queueLength)
{
    if (fFileDesc == EventContext::kInvalidFileDesc)
       return EBADF;
        
    int err = listen(fFileDesc, queueLength);
    if (err != 0)
        return Thread::GetErrno();
    
    return 0;
}

int TCPListenerSocket::Initialize(UInt32 addr, UInt16 port)
{
    int err = this->Open();
    if (0 == err) do
    {   
        this->ReuseAddr();

        err = this->Bind(addr, port);
        if (err != 0) break; // don't assert this is just a port already in use.

        //
        // Unfortunately we need to advertise a big buffer because our TCP sockets
        // can be used for incoming broadcast data. This could force the server
        // to run out of memory faster if it gets bogged down, but it is unavoidable.
        this->SetSocketRcvBufSize(96L * 1024);       
        this->InitNonBlocking(fFileDesc);
        
        err = this->Listen(kListenQueueLength);
        AssertV(err == 0, Thread::GetErrno()); 
        if (err != 0) break;
        
    } while (false);
    
    return err;
}

void TCPListenerSocket::ProcessEvent(int /*eventBits*/)
{
    //we are executing on the same thread as every other
    //socket, so whatever you do here has to be fast.
    struct sockaddr_in addr;
    socklen_t size = sizeof(addr);
    char errStr[256];

    Task* theTask = NULL;
    TCPSocket* theSocket = NULL;

    //fSocket data member of TCPSocket.
    int clientfd = accept(fFileDesc, (struct sockaddr*)&addr, &size);
    if (clientfd == -1)
    {
        //take a look at what this error is.
        int acceptError = Thread::GetErrno();
        if (acceptError == EAGAIN)
            return;

        //test acceptError = ENFILE;
        //test acceptError = EINTR;
        //test acceptError = ENOENT;

        //if these error gets returned, we're out of file desciptors, 
        //the server is going to be failing on sockets, logs, qtgroups and qtuser auth file accesses and movie files. The server is not functional.
        if (acceptError == EMFILE || acceptError == ENFILE)
        {
            printf("Out of File Descriptors. Set max connections lower and check for competing usage from other processes. Exiting.\n");
            exit (EXIT_FAILURE);
        }
        else
        {   
            errStr[sizeof(errStr) -1] = 0;
            snprintf(errStr, sizeof(errStr) -1, "accept error = %d '%s' on socket. Clean up and continue.", acceptError, strerror(acceptError)); 
            WarnV( (acceptError == 0), errStr);
            
            theTask = this->GetSessionTask(&theSocket);
            if (theTask == NULL)
                close(clientfd);
            else
                theTask->Signal(Task::kKillEvent); // just clean up the task
            
            if (theSocket)
                theSocket->fState &= ~kConnected; // turn off connected state
            
            return;
        }
    }


    theTask = this->GetSessionTask(&theSocket);
    if (theTask == NULL)
    {   //this should be a disconnect. do an ioctl call?
        close(clientfd);
        if (theSocket)
            theSocket->fState &= ~kConnected; // turn off connected state
    }
    else
    {   
        Assert(clientfd != EventContext::kInvalidFileDesc);
        
        //set options on the socket
        //we are a server, always disable nagle algorithm
        theSocket->NoDelay();
        theSocket->KeepAlive();

        theSocket->SetSocketRcvBufSize(96L * 1024L); 
        theSocket->SetSocketSndBufSize(96L * 1024L);

        //setup the socket. When there is data on the socket,
        //theTask will get an kReadEvent event
        theSocket->Set(clientfd, &addr);
        theSocket->Linger(1, 1);
        theSocket->InitNonBlocking(clientfd);
        theSocket->SetTask(theTask);
        theSocket->RequestEvent(EV_RE);

        theTask->SetThreadPicker(Task::GetBlockingTaskThreadPicker());
    }

    
    if (fSleepBetweenAccepts)
    {     
        // We are at our maximum supported sockets
        // slow down so we have time to process the active ones (we will respond with errors or service).
        // wake up and execute again after sleeping. The timer must be reset each time through
        // printf("TCPListenerSocket slowing down\n");
        this->SetIdleTimer(kTimeBetweenAcceptsInMsec); //sleep 1 second
    }
    /*
    else
    {     
        // sleep until there is a read event outstanding (another client wants to connect)
        // printf("TCPListenerSocket normal speed\n");
        this->RequestEvent(EV_RE, 0);
    }
    */
}

SInt64 TCPListenerSocket::Run()
{
    EventFlags events = this->GetEvents();
    
    //
    // ProcessEvent cannot be going on when this object gets deleted, because
    // the resolve / release mechanism of EventContext will ensure this thread
    // will block before destructing stuff.
    if (events & Task::kKillEvent)
        return -1;
        
    //This function will get called when we have run out of file descriptors.
    //All we need to do is check the listen queue to see if the situation has
    //cleared up.
    (void)this->GetEvents();
    this->ProcessEvent(Task::kReadEvent);
    return 0;
}

