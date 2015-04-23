/*
    File:       UDPSocket.h

    Contains:   Adds additional Socket functionality specific to UDP.

    
    
    
*/


#ifndef __UDPSOCKET_H__
#define __UDPSOCKET_H__

#include <sys/uio.h>
#include <sys/socket.h>

#include "Socket.h"
#include "UDPDemuxer.h"

class   UDPSocket : public Socket
{
    public:
    
        //Another socket type flag (in addition to the ones defined in Socket.h).
        //The value of this can't conflict with those!
        enum { kWantsDemuxer = 0x0100 }; // UInt32
    
        UDPSocket(Task* inTask, UInt32 inSocketType);
        virtual ~UDPSocket() { if (fDemuxer != NULL) delete fDemuxer; }

        //Open
        int Open() { return Socket::Open(SOCK_DGRAM); }

        int JoinMulticast(UInt32 inRemoteAddr);
        int LeaveMulticast(UInt32 inRemoteAddr);
        int SetTtl(UInt16 timeToLive);
        int SetMulticastInterface(UInt32 inLocalAddr);

        //returns an ERRNO
        int	SendTo(UInt32 inRemoteAddr, UInt16 inRemotePort,
                    	void* inBuffer, UInt32 inLength);
                        
        int RecvFrom(UInt32* outRemoteAddr, UInt16* outRemotePort,
                    	void* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen);
        
        //A UDP socket may or may not have a demuxer associated with it. The demuxer
        //is a data structure so the socket can associate incoming data with the proper
        //task to process that data (based on source IP addr & port)
        UDPDemuxer* GetDemuxer()    { return fDemuxer; }
        
    private:
    
        UDPDemuxer* fDemuxer;
        struct sockaddr_in  fMsgAddr;
};
#endif // __UDPSOCKET_H__

