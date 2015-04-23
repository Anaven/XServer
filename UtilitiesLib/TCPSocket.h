#ifndef __TCPSOCKET_H__
#define __TCPSOCKET_H__


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "Headers.h"
#include "Socket.h"
#include "Task.h"
#include "StrPtrLen.h"

class TCPSocket : public Socket
{
    public:

        //TCPSocket takes an optional task object which will get notified when
        //certain events happen on this socket. Those events are:
        //
        //Task::kReadEvent:               Data is currently available on the socket.
        TCPSocket(Task *notifytask, UInt32 inSocketType)
            :   Socket(notifytask, inSocketType),
            	fRemoteStr(fRemoteBuffer, kIPAddrBufSize) {}
        virtual ~TCPSocket() {}

        //Open
        int    Open() { return Socket::Open(SOCK_STREAM); }

        // Connect. Attempts to connect to the specified remote host. If this
        // is a non-blocking socket, this function may return EINPROGRESS, in which
        // case caller must wait for either an EV_RE or an EV_WR. You may call
        // CheckAsyncConnect at any time, which will return 0 if the connect
        // has completed, EINPROGRESS if it is still in progress, or an appropriate error
        // if the connect failed.
        int    Connect(UInt32 inRemoteAddr, UInt16 inRemotePort);

        //ACCESSORS:
        //Returns NULL if not currently available.
        
        UInt32      GetRemoteAddr() { return ntohl(fRemoteAddr.sin_addr.s_addr); }
        UInt16      GetRemotePort() { return ntohs(fRemoteAddr.sin_port); }
        //This function is NOT thread safe!
    	StrPtrLen*    GetRemoteAddrStr();

    protected:

        void        Set(int inSocket, struct sockaddr_in* remoteaddr);
                            
        enum
        {
            kIPAddrBufSize = 20 //UInt32
        };

        struct sockaddr_in  fRemoteAddr;
        char fRemoteBuffer[kIPAddrBufSize];
        StrPtrLen fRemoteStr;
        
        friend class TCPListenerSocket;
};
#endif // __TCPSOCKET_H__

