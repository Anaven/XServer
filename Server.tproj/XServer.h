#ifndef __XSERVER__
#define __XSERVER__

#include "Headers.h"
#include "Task.h"
#include "Mutex.h"
#include "Ref.h"
#include "Log.h"
#include "ConfigPrefsParser.h"
#include "SessionListenerSocket.h"


#define XSERVER_DEBUG 1

class XServer
{
	public:

    	XServer();
        ~XServer();

    	static XServer* GetServer() { return sServer; };

    	bool 	Initialize(ConfigPrefsParser *inConfigPrefsParser, UInt16 inPort, bool createListeners);
    	void 	InitNumThreads(UInt32 numThreads) {  fNumThreads = numThreads; };

    	void 	StartTasks();
    	void 	RestartTasks() {};
    	bool 	SetDefaultIPAddr();
    	bool 	CreateListeners(bool startListeningNow, UInt16 inPort);
		bool	SwitchPersonality();
		void	KillAllSessions() {};

    	void	SetSigInt()  						{ fSigInt = true;                     		};
    	void	SetSigTerm()                    { fSigTerm = true;                     		};

		// String preferences. Note that the pointers returned here is allocated
		// memory that you must delete!

        char*   GetErrorLogDir()			{ return ""; };
        char*   GetErrorLogName()		{ return ""; };
        char*   GetRunUserName() 		{ return ""; };
        char*   GetRunGroupName() 	{ return ""; };
        char*   GetPidFilePath() 		{ return ""; };
		

    	Log*	GetLog()                    	{ return fErrorLog;                    		};

    	int 	GetServerState()              	{ return fServerState;                 		};
    	Mutex* 	GetMutex()              		{ return &fMutex;                    		};
    	UInt32	GetDefaultIP()        			{ return fDefaultIPAddr;            		};
    	UInt32	GetMaxOberver()             	{ return fMaxObserver;                		};

        UInt32 	GetMessagePacketSize()     		{ return fMessagePacketSize;        		};
    	UInt32 	GetMaxMessagePackets()      	{ return fMaxMessagePackets;          		};
		
    	UInt32 	GetAudioPacketSize()            	{ return fAudioPacketSize;           		};
    	UInt32 	GetMaxAudioPackets()        	{ return fMaxAudioPackets;          		};
		
    	UInt32 	GetVideoPacketSize()           	{ return fVideoPacketSize;              	};
    	UInt32 	GetMaxVideoPackets()          	{ return fMaxVideoPackets;              	};

    	UInt16	GetWebPort()               		{ return fWebPort;                    		};
    	char*	GetWebIP()                 		{ return fWebIP;                    		};
    	char*	GetServerID()            		{ return fServerID;                    		};

    	UInt16  GetMessagePort()             	{ return fMessagePort;                 		};
    	UInt16  GetAudioPort()              		{ return fAudioPort;                 		};
    	UInt16  GetVideoPort()              		{ return fVideoPort;                 		};

    	UInt32 	GetKeepAliveInMilSec()        	{ return fKeepAliveInMilSec;            	};
		UInt32 	GetSynTimeIntervalInMilSec() 		{ return fSynTimeIntervalMilSec; 			};

    	UInt32 	GetNumShortThreads()        	{ return fNumShortThreads;         			};
    	UInt32 	GetNumBlockingThreads()        	{ return fNumBlockingThreads;     			};
    	SInt32  GetMaxConnections()            	{ return fMaxConnections;         			};

    	int 	WriteLog(const char* fmt, ...);
        
		void 	ServerStateReport();
		void 	DevicesStateReport();
		void 	StartSynchronizeTimeSession();

    	enum
        {
            kStartingUpState             = 0,
            kRunningState                = 1,
            kRefusingConnectionsState    = 2,
            kFatalErrorState             = 3,//a fatal error has occurred, not shutting down yet
            kShuttingDownState           = 4,
            kIdleState                   = 5, // Like refusing connections state, but will also kill any currently connected clients
        };
        
	private:

    	Mutex 	fMutex;

    	UInt32 	fServerState;
    	UInt32	fDefaultIPAddr;
    	bool	fSigInt;
    	bool	fSigTerm;
        
    	UInt32  fDebugLevel;
    	UInt32 	fMaxObserver;

    	UInt32 	fNumThreads;
    	UInt32 	fNumListeners;

    	UInt32 	fNumShortThreads;
    	UInt32 	fNumBlockingThreads;

    	SInt32	fMaxConnections;

    	UInt16 	fMessagePort;
    	UInt16 	fAudioPort;
    	UInt16 	fVideoPort;

    	UInt32 	fMessagePacketSize;
		UInt32 	fMaxMessagePackets;

    	UInt32 	fAudioPacketSize;
    	UInt32	fMaxAudioPackets;

    	UInt32 	fVideoPacketSize;
    	UInt32	fMaxVideoPackets;
            
    	UInt32 	fKeepAliveInMilSec;
		UInt32	fSynTimeIntervalMilSec;

    	UInt32	fNumMessageSession;
    	UInt32	fNumAudioSession;
    	UInt32	fNumVideoSession;

    	UInt32 	fProcessNum;
    	UInt16 	fWebPort;
    	char	fWebIP[kIPSize];
    	char    fServerID[kServerIDSize];
		char	fLogPath[kFilePathSize];

    	SInt64	fStartupTime_UnixMilli;
    	SInt32	fGMTOffset;

    	Log*	fErrorLog;

    	TCPListenerSocket* fMessageListener;
		
    	static XServer* sServer;
};

#endif
