#ifndef __HTTP_SESSION__
#define __HTTP_SESSION__

#include "Task.h"
#include "Headers.h"
#include "Mutex.h"
#include "TCPSocket.h"
#include "SocketUtils.h"
#include "KeepAliveTask.h"


#define HTTP_SESSION_DEBUG 1

class HttpSession : public Task
{
	public:

    	HttpSession(int theMethod);
        virtual ~HttpSession();

    	void 	Cleanup() {};
    	SInt64 	Run();
    	bool 	IsLiveSession() { return fLiveSession; };

    	int 	ConnectToServer();
    	SInt64 	DoTranscation();

		void 	ConstructHeader(char* theURI, char* theWebIP, UInt16 theWebPort);
		void 	ConstructPostHeader(char* inURI, char* theWebIP, UInt16 theWebPort);
		void 	ConstructGetHeader(char* inURI, char* theWebIP, UInt16 theWebPort);
		int 	ConstructContent(const char* theName, const char* theType, char* inData, 
								UInt32 inDataLen, bool appendBoundary = false);
		
    	virtual int ProcessRespond() = 0;

	protected:

    	enum
        {
        	kSendingHeader		= 0,
        	kSendingContent 	= 1,
        	kSendingEnd			= 2,
			kPocessHeader		= 3,
        	kProcessRespond 	= 4,
        	kCleanup	    	= 5,
        };

		enum
		{
			kHttpSuccess	= 0x00,
			kHttpProcessing	= 0x01,
			kHttpFailed		= 0x02,
		};

    	enum 
    	{ 
    	    kIdleTimeInMilSec 	= 10,	
			kTimeoutInMilSec	= 30000,		
    	};
		
		enum 
		{ 
			kHttpHeaderSize     = 3072,
			kHttpEndSize        = 256,
			kHttpContentSize    = 1024000,
			kBufferSize			= 2048,
			kURLSize 			= 2048,
		};

		enum
		{
			kHttpGet 	= 1,
			kHttpPost 	= 2,
		};

    	TCPSocket		fSocket;
    	Mutex       	fMutex;
		KeepAliveTask 	fKeepAliveTask;

    	bool         	fLiveSession;
    	int         	fState;
		int 			fMethod;
		int				fHttpCode;

		char*			fContent;
		UInt32			fContentLength;

		char			fURL[kURLSize];
    	char	    	fRecvBuffer[kBufferSize];
    	UInt32	    	fRecvBufferLen;

		char*			fHttpHeader;
		UInt32			fHttpHeaderLen;
		UInt32			fHttpHeaderStart;
		
    	char*	    	fHttpContent;
    	UInt32	    	fHttpContentLen;
    	UInt32	    	fHttpContentStart;
    	
    	char*	    	fHttpEnd;
    	UInt32	    	fHttpEndLen;
    	UInt32	    	fHttpEndStart;

		static const char*  sBoundary;
		static const char*  sBoundaryStart;
		static const char*	sBoundaryEnd;
};

#endif
