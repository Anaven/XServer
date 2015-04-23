#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <netinet/in.h>

#include "OS.h"
#include "Headers.h"
#include "EventContext.h"

#define SOCKET_DEBUG 0

class Socket : public EventContext
{
    public:
    
         enum { kNonBlockingSocketType = 1 };

        //This class provides a global event thread.
        static void         	Initialize() { sEventThread = new EventThread(); }
        static void         	StartThread() 
		{ 
			sEventThread->Start(); 
			UInt32 theCPUs = OS::GetNumProcessors();
			sEventThread->SetAffinity(theCPUs - 1); 
		}
        static EventThread* 	GetEventThread() { return sEventThread; }
        

        int		Bind(UInt32 addr, UInt16 port);
        void 	Unbind();   
        
        void	ReuseAddr();
        void	NoDelay();
        void	KeepAlive();
    	void	Linger(SInt32 onoff, SInt32 sec);
        int		SetSocketSndBufSize(UInt32 inNewSize);

        //Returns an error if the socket buffer size is too big
        int		SetSocketRcvBufSize(UInt32 inNewSize);
        
        int		Send(const char* inData, const UInt32 inLength, UInt32* outLengthSent);
        int		Read(void *buffer, const UInt32 length, UInt32 *rcvLen);      
        int		WriteV(const struct iovec* iov, const UInt32 numIOvecs, UInt32* outLengthSent);
        
        bool  	IsConnected()   { return (fState & kConnected); }
        bool  	IsBound()       { return (fState & kBound); }
        
        UInt32     	GetLocalAddr()  { return ntohl(fLocalAddr.sin_addr.s_addr); }
        UInt16     	GetLocalPort()  { return ntohs(fLocalAddr.sin_port); }
      
        enum { kMaxNumSockets = 4096 };

    protected:

        //TCPSocket takes an optional task object which will get notified when
        //certain events happen on this socket. Those events are:
        //
        //S_DATA:               Data is currently available on the socket.
        //S_CONNECTIONCLOSING:  Client is closing the connection. No longer necessary
        //                      to call Close or Disconnect, Snd & Rcv will fail.
        
        Socket(Task *notifytask, int inSocketType);
        virtual ~Socket() {}

        int	Open(int theType);
        
        int fState;
        
        enum
        {
            kPortBufSizeInBytes = 8,    //SInt32
            kMaxIPAddrSizeInBytes = 20  //SInt32
        };
        
        //address information (available if bound)
        //these are always stored in network order. Conver
        struct sockaddr_in  fLocalAddr;
        struct sockaddr_in  fDestAddr;
        
        enum
        {
            kBound      = 0x0004,
            kConnected  = 0x0008
        };
        
        static EventThread* sEventThread;
};

#endif // __SOCKET_H__

