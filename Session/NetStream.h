#ifndef __NET_STREAM__
#define __NET_STREAM__

#include "Ref.h"
#include "TCPSocket.h"
#include "Mutex.h"


#define NET_STREAM_DEBUG 1

class NetStream : public Task
{
	public:
		
    	NetStream();
        ~NetStream();

      	void 	Cleanup();
    	SInt64 	Run();

		TCPSocket* GetSocket() { return &fSocket; };

		enum
		{
			kSetup				= 0,
			kProcessClient		= 1,
			kCleanup			= 2,
		};

		enum
		{
			kBufSize = 1024,
		};
				
	private:

		enum
		{
			kIdleTimeInMilSec = 10,
		};

	    TCPSocket	fSocket;
		int			fFileSource;

		char		fRecvBuffer[kBufSize];

		static UInt32 	sCount;
		static SInt64	sStart;
		static SInt64	sEnd;
};

#endif

