#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include "TCPSocket.h"
#include "SocketUtils.h"
#include "OS.h"


void TCPSocket::Set(int inSocket, struct sockaddr_in* remoteaddr)
{
    fRemoteAddr = *remoteaddr;
    fFileDesc = inSocket;
    
    if ( inSocket != EventContext::kInvalidFileDesc ) 
    {
        //make sure to find out what IP address this connection is actually occuring on. That
        //way, we can report correct information to clients asking what the connection's IP is
        socklen_t len = sizeof(fLocalAddr);

        int err = ::getsockname(fFileDesc, (struct sockaddr*)&fLocalAddr, &len);
        AssertV(err == 0, Thread::GetErrno());
        fState |= kBound;
        fState |= kConnected;
    }
    else
        fState = 0;
}

StrPtrLen*  TCPSocket::GetRemoteAddrStr()
{
    if (fRemoteStr.Len == kIPAddrBufSize)
        SocketUtils::ConvertAddrToString(fRemoteAddr.sin_addr, &fRemoteStr);
    return &fRemoteStr;
}

int TCPSocket::Connect(UInt32 inRemoteAddr, UInt16 inRemotePort)
{
    ::memset(&fRemoteAddr, 0, sizeof(fRemoteAddr));
    fRemoteAddr.sin_family = AF_INET;        /* host byte order */
    fRemoteAddr.sin_port = htons(inRemotePort); /* short, network byte order */
    fRemoteAddr.sin_addr.s_addr = htonl(inRemoteAddr);

    /* don't forget to error check the connect()! */
    int err = ::connect(fFileDesc, (sockaddr *)&fRemoteAddr, sizeof(fRemoteAddr));
    fState |= kConnected;
    
    if (err == -1)
    {
        fRemoteAddr.sin_port = 0;
        fRemoteAddr.sin_addr.s_addr = 0;
        return Thread::GetErrno();
    }
    
    return 0;
}

