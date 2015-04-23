/*
    File:       UDPSocket.cpp

    Contains:   Implementation of object defined in UDPSocket.h.

    
    
*/


#include <sys/types.h>
#include <sys/socket.h>


#if NEED_SOCKETBITS
#if __GLIBC__ >= 2
#include <bits/socket.h>
#else
#include <socketbits.h>
#endif
#endif

#include <errno.h>

#include "Memory.h"
#include "UDPSocket.h"

UDPSocket::UDPSocket(Task* inTask, UInt32 inSocketType)
: Socket(inTask, inSocketType), fDemuxer(NULL)
{
    if (inSocketType & kWantsDemuxer)
        fDemuxer = NEW UDPDemuxer();
        
    //setup msghdr
    ::memset(&fMsgAddr, 0, sizeof(fMsgAddr));
}


int UDPSocket::SendTo(UInt32 inRemoteAddr, UInt16 inRemotePort, void* inBuffer, UInt32 inLength)
{
    Assert(inBuffer != NULL);
    
    struct sockaddr_in  theRemoteAddr;
    theRemoteAddr.sin_family = AF_INET;
    theRemoteAddr.sin_port = htons(inRemotePort);
    theRemoteAddr.sin_addr.s_addr = htonl(inRemoteAddr);

	int theErr = ::sendto(fFileDesc, inBuffer, inLength, 0, (sockaddr*)&theRemoteAddr, sizeof(theRemoteAddr));

    if (theErr == -1)
        return Thread::GetErrno();
    
    return 0;
}

int UDPSocket::RecvFrom(UInt32* outRemoteAddr, UInt16* outRemotePort,
                            void* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen)
{
    Assert(outRecvLen != NULL);
    Assert(outRemoteAddr != NULL);
    Assert(outRemotePort != NULL);
    
    socklen_t addrLen = sizeof(fMsgAddr);

	SInt32 theRecvLen = ::recvfrom(fFileDesc, ioBuffer, inBufLen, 0, (sockaddr*)&fMsgAddr, &addrLen);

    if (theRecvLen == -1)
        return Thread::GetErrno();
    
    *outRemoteAddr = ntohl(fMsgAddr.sin_addr.s_addr);
    *outRemotePort = ntohs(fMsgAddr.sin_port);
    Assert(theRecvLen >= 0);
    *outRecvLen = (UInt32)theRecvLen;
    return 0;        
}

int UDPSocket::JoinMulticast(UInt32 inRemoteAddr)
{
    struct ip_mreq  theMulti;
    UInt32 localAddr = fLocalAddr.sin_addr.s_addr; // Already in network byte order

    theMulti.imr_multiaddr.s_addr = htonl(inRemoteAddr);
    theMulti.imr_interface.s_addr = localAddr;
    int err = setsockopt(fFileDesc, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&theMulti, sizeof(theMulti));
    //AssertV(err == 0, Thread::GetErrno());
    if (err == -1)
         return Thread::GetErrno();
    else
         return 0;
}

int UDPSocket::SetTtl(UInt16 timeToLive)
{
    // set the ttl
    u_char  nOptVal = (u_char)timeToLive;//dms - stevens pp. 496. bsd implementations barf
                                            //unless this is a u_char
    int err = setsockopt(fFileDesc, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&nOptVal, sizeof(nOptVal));
    if (err == -1)
        return Thread::GetErrno();
    else
        return 0;    
}

int UDPSocket::SetMulticastInterface(UInt32 inLocalAddr)
{
    // set the outgoing interface for multicast datagrams on this socket
    in_addr theLocalAddr;
    theLocalAddr.s_addr = inLocalAddr;
    int err = setsockopt(fFileDesc, IPPROTO_IP, IP_MULTICAST_IF, (char*)&theLocalAddr, sizeof(theLocalAddr));
    AssertV(err == 0, Thread::GetErrno());
    if (err == -1)
        return Thread::GetErrno();
    else
        return 0;    
}

int UDPSocket::LeaveMulticast(UInt32 inRemoteAddr)
{
    struct ip_mreq  theMulti;
    theMulti.imr_multiaddr.s_addr = htonl(inRemoteAddr);
    theMulti.imr_interface.s_addr = htonl(fLocalAddr.sin_addr.s_addr);
    int err = setsockopt(fFileDesc, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&theMulti, sizeof(theMulti));
    if (err == -1)
        return Thread::GetErrno();
    else
        return 0;    
}
