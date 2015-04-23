#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#include "NetStream.h"
#include "XServer.h"
#include "OS.h"


UInt32 NetStream::sCount = 0;
SInt64 NetStream::sStart = 0;
SInt64 NetStream::sEnd = 0;

NetStream::NetStream() 
 :  fSocket(NULL, Socket::kNonBlockingSocketType),
    fFileSource(-1)
{
    ++sCount;
    printf("Client %u In\n", sCount);
};


NetStream::~NetStream() 
{
    --sCount;
    printf("Client %u Out\n", sCount);
    if (fFileSource != -1) 
    {
        ::close(fFileSource);
    }
};


SInt64 NetStream::Run()
{
    EventFlags theEvents = this->GetEvents();

    if (theEvents & Task::kKillEvent || theEvents & Task::kTimeoutEvent)
    {
        return -1;
    }
    
    if (theEvents & Task::kReadEvent)
    {
        UInt32 theLen = 0;
        int err = 0;
        while (err != EAGAIN)
        {
            theLen = 0;
            err = fSocket.Read(fRecvBuffer, kBufSize, &theLen);
            if (err != 0 && err != EAGAIN)
            {
                //printf("NetStream[%p]: read close\n", this);
                return -1;
            }

            //printf("NetStream[%p]: read %u nytes\n", this, theLen);
        }

        fFileSource = open("./index.html", O_RDONLY | O_NONBLOCK);
        Assert(fFileSource != -1);

        int sockfd = fSocket.GetSocketFD();
        int nBytes = ::sendfile(sockfd, fFileSource, NULL, kBufSize);

        ::close(fFileSource);
        if (nBytes == -1)
        {
            //printf("NetStream[%p]: sendfile error\n", this);
            return -1;
        }
        //printf("NetStream[%p]: send %u nytes\n", this, nBytes);

        return 0;
    }

    return 0;
}

