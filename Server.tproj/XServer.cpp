#include <stdarg.h>
#include <sys/mman.h>

#include "OS.h"
#include "Queue.h"
#include "Packet.h"
#include "XServer.h"
#include "SocketUtils.h"
#include "EventContext.h"
#include "NetStream.h"

XServer* XServer::sServer = NULL;

XServer::XServer() 
 :  fServerState(kStartingUpState),
    fDefaultIPAddr(0),
    fSigInt(false),
    fSigTerm(false),
    fDebugLevel(0),
    fMaxObserver(0),
    fNumThreads(0),
    fNumListeners(0),
    fNumShortThreads(0),
    fNumBlockingThreads(0),
    fMaxConnections(0),
    fMessagePort(0),
    fAudioPort(0),
    fVideoPort(0),
    fMessagePacketSize(128),
    fMaxMessagePackets(10),
    fAudioPacketSize(1024),
    fMaxAudioPackets(10),
    fVideoPacketSize(40960),
    fMaxVideoPackets(5),
    fKeepAliveInMilSec(0),
    fSynTimeIntervalMilSec(0),
    fNumMessageSession(0),
    fNumAudioSession(0),
    fNumVideoSession(0),
    fProcessNum(0),
    fWebPort(0),
    fStartupTime_UnixMilli(0),
    fGMTOffset(0),
    fErrorLog(NULL),
    fMessageListener(NULL)
{ 
    memset(fWebIP, 0 , kIPSize);
    memset(fServerID, 0, kServerIDSize);
    memset(fLogPath, 0, kFilePathSize);

	sServer = this;
};

XServer::~XServer() 
{  
    if (fMessageListener)
    {
    	fMessageListener->Cleanup();
    	delete fMessageListener;
        fMessageListener = NULL;
    }
};

bool XServer::Initialize(ConfigPrefsParser *inConfigPrefsParser, UInt16 inPort, bool createListeners)
{
    fServerState                        = kFatalErrorState;

    fNumShortThreads                    = inConfigPrefsParser->GetShortTaskThreadsNum();
    fNumBlockingThreads                 = inConfigPrefsParser->GetBlockingThreadsNum();

    fMaxConnections                     = inConfigPrefsParser->GetMaxConnections();

    fMessagePort                        = inConfigPrefsParser->GetMessagePort();
    fAudioPort                          = inConfigPrefsParser->GetAudioPort();
    fVideoPort                          = inConfigPrefsParser->GetVideoPort();

    fMessagePacketSize                  = inConfigPrefsParser->GetMessagePacketSize();
    fMaxMessagePackets                  = inConfigPrefsParser->GetMaxMessagePackets();

    fAudioPacketSize                    = inConfigPrefsParser->GetAudioPacketSize(); 
    fMaxAudioPackets                    = inConfigPrefsParser->GetMaxAudioPackets();
    
    fVideoPacketSize                     = inConfigPrefsParser->GetVideoPacketSize(); 
    fMaxVideoPackets                     = inConfigPrefsParser->GetMaxVideoPackets();

    fKeepAliveInMilSec                  = inConfigPrefsParser->GetKeepAlive() * 1000; 
    fSynTimeIntervalMilSec              = inConfigPrefsParser->GetSynchronizeTime() * 1000; 
    
    fProcessNum                         = inConfigPrefsParser->GetProcessNum();
    fWebPort                            = inConfigPrefsParser->GetWebPort();

    fMaxObserver	                    = inConfigPrefsParser->GetMaxObserver();

    strncpy(fWebIP, inConfigPrefsParser->GetWebIP(), kIPSize);
    strncpy(fServerID, inConfigPrefsParser->GetServerID(), kServerIDSize);
    strncpy(fLogPath, inConfigPrefsParser->GetLogPath(), kFilePathSize);

    printf("fNumShortThreads = %u\n", fNumShortThreads);
    printf("fNumBlockingThreads = %u\n", fNumBlockingThreads);
    printf("fMaxConnections = %u\n", fMaxConnections);
    printf("fMessagePort = %hu\n", fMessagePort);
    printf("fAudioPort = %hu\n", fAudioPort);
    printf("fVideoPort = %hu\n", fVideoPort );

    printf("fMessagePacketSize = %u\n", fMessagePacketSize);
    printf("fMaxMessagePackets = %u\n", fMaxMessagePackets);

    printf("fAudioPacketSize = %u\n", fAudioPacketSize);
    printf("fMaxAudioPackets = %u\n", fMaxAudioPackets);

    printf("fVideoPacketSize = %u\n", fVideoPacketSize);
    printf("fMaxVideoPackets = %u\n", fMaxVideoPackets);

    printf("fKeepAliveInMilSec = %u\n", fKeepAliveInMilSec);
    printf("fSynTimeIntervalMilSec = %u\n", fSynTimeIntervalMilSec);

    printf("fProcessNum = %u\n", fProcessNum);
    printf("fWebPort = %hu\n", fWebPort);
    printf("fMaxObserver = %u\n", fMaxObserver);

    printf("fWebIP = %s\n", fWebIP);
    printf("fServerID = %s\n", fServerID);
    printf("fLogPath = %s\n", fLogPath);

    //
    // SETUP ASSERT BEHAVIOR
    //
    // Depending on the server preference, we will either break when we hit an
    // assert, or log the assert to the error log

    // DEFAULT IP ADDRESS & DNS NAME
    if (!this->SetDefaultIPAddr())
        return false;

    fStartupTime_UnixMilli = OS::Milliseconds();
    fGMTOffset = OS::GetGMTOffset();

    //
    // BEGIN LISTENING
    if (createListeners)
    {
        this->CreateListeners(true, inPort);
    }
    
    if (fNumListeners < 1)
        return false;
    
    fServerState = kStartingUpState;
    return true;
}

bool XServer::SetDefaultIPAddr()
{
    //check to make sure there is an available ip interface
    if (SocketUtils::GetNumIPAddrs() == 0)
        return false;

    //find out what our default IP addr is & dns name
    fDefaultIPAddr = SocketUtils::GetIPAddr(0);
    
    return true;
}   

bool XServer::CreateListeners(bool startListeningNow, UInt16 inPort)
{
    if (fMessageListener == NULL)
    {            
        fMessageListener = new MessageListenerSocket;
        int err = fMessageListener->Initialize(INADDR_ANY, fMessagePort);

        //
        // If there was an error creating this listener, destroy it and log an error
        if ((startListeningNow) && (err != 0))
            delete fMessageListener;

        if (err == EADDRINUSE)
            printf("EADDRINUSE\n");
        else if (err == EACCES)
            printf("EACCES\n");
        else if (err != 0)
            printf("ERROR\n");
        else
        {
            //
            // This listener was successfully created.
            if (startListeningNow) 
                fMessageListener->RequestEvent(EV_RE, 0);

            fNumListeners++;
        }
    }

    return (fNumListeners >= 1);
}

void XServer::StartTasks()
{
    if (fMessageListener) fMessageListener->RequestEvent(EV_RE, 0);

    fServerState = kStartingUpState;
}

bool  XServer::SwitchPersonality()
{
   return true;
}

int XServer::WriteLog(const char* fmt, ...)
{
    MutexLocker theLocker(&fMutex);
    Log *theLog = XServer::GetServer()->GetLog();

    char theLogData[kLogDataSize];
    va_list args;
    ::va_start(args, fmt);
    int result = ::vsprintf(theLogData, fmt, args);
    ::va_end(args);

    theLog->LogInfo(theLogData);

    #if XSERVER_DEBUG
        printf("%s\n", theLogData);
    #endif
          
    return result;
}

