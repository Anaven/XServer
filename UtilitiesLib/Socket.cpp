/*
    File:       Socket.cpp

    Contains:   implements Socket class
                    

    
*/
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <errno.h>

#include "MyAssert.h"
#include "Socket.h"
#include "Memory.h"


EventThread* Socket::sEventThread = NULL;

Socket::Socket(Task *notifytask, int inSocketType)
:   EventContext(EventContext::kInvalidFileDesc, sEventThread),
    fState(inSocketType)
{
    fLocalAddr.sin_addr.s_addr = 0;
    fLocalAddr.sin_port = 0;
    
    fDestAddr.sin_addr.s_addr = 0;
    fDestAddr.sin_port = 0;

    this->SetTask(notifytask);
}


int Socket::Open(int theType)
{
    Assert(fFileDesc == EventContext::kInvalidFileDesc);
    fFileDesc = ::socket(AF_INET, theType, 0);
    if (fFileDesc == EventContext::kInvalidFileDesc)
        return Thread::GetErrno();
            
    if (fState & kNonBlockingSocketType)
        this->InitNonBlocking(fFileDesc);   

    return 0;
}


void Socket::ReuseAddr()
{
    int one = 1;
    int err = ::setsockopt(fFileDesc, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(int));
    Assert(err == 0);   
}


void Socket::NoDelay()
{
    int one = 1;
    int err = ::setsockopt(fFileDesc, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int));
    Assert(err == 0);   
}


void Socket::KeepAlive()
{
    int one = 1;
    int err = ::setsockopt(fFileDesc, SOL_SOCKET, SO_KEEPALIVE, (char*)&one, sizeof(int));
    Assert(err == 0);   
}


void Socket::Linger(SInt32 onoff, SInt32 sec)
{
	struct linger linger;

	linger.l_onoff = onoff;
	linger.l_linger = sec;

	int err = ::setsockopt(fFileDesc, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(struct linger));
	Assert(err == 0);
}


int Socket::SetSocketSndBufSize(UInt32 inNewSize)
{
    int bufSize = inNewSize;
    int err = ::setsockopt(fFileDesc, SOL_SOCKET, SO_SNDBUF, (char*)&bufSize, sizeof(int));
    
    if (err == -1)
        return Thread::GetErrno();
        
    return 0;

}


int Socket::SetSocketRcvBufSize(UInt32 inNewSize)
{
    int bufSize = inNewSize;
    int err = ::setsockopt(fFileDesc, SOL_SOCKET, SO_RCVBUF, (char*)&bufSize, sizeof(int));

    if (err == -1)
        return Thread::GetErrno();
        
    return 0;
}


int Socket::Bind(UInt32 addr, UInt16 port)
{
    socklen_t len = sizeof(fLocalAddr);
    ::memset(&fLocalAddr, 0, sizeof(fLocalAddr));
    fLocalAddr.sin_family = AF_INET;
    fLocalAddr.sin_port = htons(port);
    fLocalAddr.sin_addr.s_addr = htonl(addr);
    
    int err;
    
    err = ::bind(fFileDesc, (sockaddr *)&fLocalAddr, sizeof(fLocalAddr));

    if (err == -1)
    {
        fLocalAddr.sin_port = 0;
        fLocalAddr.sin_addr.s_addr = 0;
        return Thread::GetErrno();
    }
    else 
        ::getsockname(fFileDesc, (sockaddr *)&fLocalAddr, &len); // get the kernel to fill in unspecified values
    fState |= kBound;
    return 0;
}

int Socket::Send(const char* inData, const UInt32 inLength, UInt32* outLengthSent)
{
    Assert(inData != NULL);
    
    if (!(fState & kConnected))
        return ENOTCONN;
        
    int err;
    do {
       err = ::send(fFileDesc, inData, inLength, 0);//flags??
    } while((err == -1) && (Thread::GetErrno() == EINTR));
    if (err == -1)
    {
        //Are there any errors that can happen if the client is connected?
        //Yes... EAGAIN. Means the socket is now flow-controleld
        int theErr = Thread::GetErrno();
        if ((theErr != EAGAIN) && (this->IsConnected()))
            fState ^= kConnected;//turn off connected state flag
        return theErr;
    }
    
    *outLengthSent = err;
    return 0;
}

int Socket::WriteV(const struct iovec* iov, const UInt32 numIOvecs, UInt32* outLenSent)
{
    Assert(iov != NULL);

    if (!(fState & kConnected))
        return ENOTCONN;
        
    int err;
    do {
       err = ::writev(fFileDesc, iov, numIOvecs);//flags??
    } while((err == -1) && (Thread::GetErrno() == EINTR));
    if (err == -1)
    {
        // Are there any errors that can happen if the client is connected?
        // Yes... EAGAIN. Means the socket is now flow-controleld
        int theErr = Thread::GetErrno();
        if ((theErr != EAGAIN) && (this->IsConnected()))
            fState ^= kConnected;//turn off connected state flag
        return theErr;
    }
    if (outLenSent != NULL)
        *outLenSent = (UInt32)err;
        
    return 0;
}

int Socket::Read(void *buffer, const UInt32 length, UInt32 *outRecvLenP)
{
    Assert(outRecvLenP != NULL);
    Assert(buffer != NULL);

    if (!(fState & kConnected))
        return ENOTCONN;
            
    //int theRecvLen = ::recv(fFileDesc, buffer, length, 0);//flags??
    int theRecvLen;
    do {
       theRecvLen = ::recv(fFileDesc, (char*)buffer, length, 0);//flags??
    } while((theRecvLen == -1) && (Thread::GetErrno() == EINTR));

    if (theRecvLen == -1)
    {
        // Are there any errors that can happen if the client is connected?
        // Yes... EAGAIN. Means the socket is now flow-controleld
        int theErr = Thread::GetErrno();
        if ((theErr != EAGAIN) && (this->IsConnected()))
            fState ^= kConnected;//turn off connected state flag
        return theErr;
    }
    //if we get 0 bytes back from read, that means the client has disconnected.
    //Note that and return the proper error to the caller
    else if (theRecvLen == 0)
    {
        fState ^= kConnected;
        return ENOTCONN;
    }
    Assert(theRecvLen > 0);
    *outRecvLenP = (UInt32)theRecvLen;
    return 0;
}
