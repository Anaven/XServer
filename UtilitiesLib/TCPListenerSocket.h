/*
    File:       TCPListenerSocket.h

    Contains:   A TCP listener socket. When a new connection comes in, the listener
                attempts to assign the new connection to a socket object and a Task
                object. Derived classes must implement a method of getting new
                Task & socket objects
                    
    
*/
#ifndef __TCPLISTENERSOCKET_H__
#define __TCPLISTENERSOCKET_H__

#include "IdleTask.h"
#include "TCPSocket.h"

class TCPListenerSocket : public TCPSocket, public IdleTask
{
    public:

        TCPListenerSocket() :   TCPSocket(NULL, Socket::kNonBlockingSocketType),
                                fAddr(0), fPort(0), fOutOfDescriptors(false), fSleepBetweenAccepts(false) { this->SetTaskName("TCPListenerSocket"); }
        virtual ~TCPListenerSocket() {}

        //addr = listening address. port = listening port. Automatically
        //starts listening                       
        int	Initialize(UInt32 addr, UInt16 port);

        //You can query the listener to see if it is failing to accept
        //connections because the OS is out of descriptors.
        Bool16      IsOutOfDescriptors() { return fOutOfDescriptors; }

        void        SlowDown() { fSleepBetweenAccepts = true; }
        void        RunNormal() { fSleepBetweenAccepts = false; }        
        //derived object must implement a way of getting tasks & sockets to this object 
        virtual Task*   GetSessionTask(TCPSocket** outSocket) = 0;
        
        virtual SInt64  Run();
            
    private:
    
        enum
        {
            kTimeBetweenAcceptsInMsec = 1000,   //UInt32
            kListenQueueLength = 1024           //UInt32
        };

        virtual void 	ProcessEvent(int eventBits = 0);
        int         	Listen(UInt32 queueLength);
    
	private:

        UInt32  	fAddr;
        UInt16   	fPort;

		bool		fOutOfDescriptors;
    	bool      	fSleepBetweenAccepts;
};

#endif // __TCPLISTENERSOCKET_H__

