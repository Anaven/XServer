#include <stdarg.h>
#include <sys/mman.h>

#include "HttpSession.h"
#include "XServer.h"


const char* HttpSession::sBoundary 		= 	"----WebKitFormBoundary3fOzWLBLMrnh8eqK";
const char*	HttpSession::sBoundaryStart = 	"------WebKitFormBoundary3fOzWLBLMrnh8eqK";
const char*	HttpSession::sBoundaryEnd 	= 	"------WebKitFormBoundary3fOzWLBLMrnh8eqK--\r\n\r\n";

HttpSession::HttpSession(int theMethod) 
 :  fSocket(NULL, Socket::kNonBlockingSocketType),
    fMutex(),
    fKeepAliveTask(this),
	fLiveSession(true),
	fState(kSendingHeader),
	fMethod(theMethod),
	fHttpCode(0),
	fContent(NULL),
	fContentLength(0),
    fRecvBufferLen(0),
    fHttpHeader(NULL),
    fHttpHeaderLen(0),
    fHttpHeaderStart(0),
    fHttpContent(NULL),
    fHttpContentLen(0),
    fHttpContentStart(0),
    fHttpEnd(NULL),
    fHttpEndLen(0),
    fHttpEndStart(0)
{ 
	this->SetTaskName("http");
	fKeepAliveTask.SetTimeout(kTimeoutInMilSec);

	fHttpHeader 	= NEW char[kHttpHeaderSize];
	fHttpContent 	= NEW char[kHttpContentSize];
	fHttpEnd 		= NEW char[kHttpEndSize];
};


HttpSession::~HttpSession()
{
    delete [] fHttpHeader;
	delete [] fHttpContent;
	delete [] fHttpEnd;
}


void HttpSession::ConstructHeader(char* inURI, char* theWebIP, UInt16 theWebPort)
{
	if (fMethod == kHttpGet)
	{
		this->ConstructGetHeader(inURI, theWebIP, theWebPort);
	}
	else if (fMethod == kHttpPost)
	{
		this->ConstructPostHeader(inURI, theWebIP, theWebPort);
	}
	else
		Assert(0);
}


void HttpSession::ConstructGetHeader(char* inURI, char* theWebIP, UInt16 theWebPort)
{
    snprintf(fHttpHeader, kHttpHeaderSize, "GET %s HTTP/1.1\r\n" 	\
                                        "Host: %s:%d\r\n\r\n", inURI, theWebIP, theWebPort);
    fHttpHeaderLen = strlen(fHttpHeader);
}


void HttpSession::ConstructPostHeader(char* inURI, char* theWebIP, UInt16 theWebPort)
{
	UInt32 theBoundaryEndLen = strlen(sBoundaryEnd);
	Assert(theBoundaryEndLen <= kHttpEndSize);
	memcpy(fHttpEnd, sBoundaryEnd, theBoundaryEndLen);
	fHttpEndLen = theBoundaryEndLen;


	UInt32 theContentLen = fHttpContentLen + fHttpEndLen;
	snprintf(fHttpHeader, kHttpHeaderSize, "POST %s HTTP/1.1\r\n"				\
										"Host: %s:%hu\r\n"						\
										"Connection: keep-alive\r\n"			\
										"Content-Length: %u\r\n"				\
										"Cache-Control: max-age=0\r\n"			\
										"Origin: http://%s:%hu\r\n"				\
										"User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/535.19 (KHTML, like Gecko) Chrome/18.0.1025.162 Safari/535.19\r\n"	\
										"Content-Type: multipart/form-data; boundary=%s\r\n\r\n"								\
										, inURI, theWebIP, theWebPort, theContentLen, theWebIP, theWebPort, sBoundary);
	fHttpHeaderLen = strlen(fHttpHeader);
	Assert(fHttpHeaderLen <= kHttpHeaderSize);
}


int HttpSession::ConstructContent(const char* theName, const char* theType, 
									char* inData, UInt32 inDataLen, bool appendBoundary)
{
	UInt32 theBlockHeaderLen = 0;

	
	if (appendBoundary)
	{
		char theBlockHeader[256];
		snprintf(theBlockHeader, 256, "%s\r\n"																		\
									"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"	\
									"Content-Type: %s\r\n\r\n", sBoundaryStart, theName, theName, theType);
		theBlockHeaderLen = strlen(theBlockHeader);

		if (fHttpContentLen + theBlockHeaderLen + inDataLen + 4 > kHttpContentSize)
		{
			printf("HttpSession[%p]: No Enough ContentSize.\n", this);
			return -1;
		}

		memcpy(&fHttpContent[fHttpContentLen], theBlockHeader, theBlockHeaderLen);
		fHttpContentLen += theBlockHeaderLen;
	}
	else
	{
		if (fHttpContentLen + inDataLen + 4 > kHttpContentSize)
		{
			printf("HttpSession[%p]: No Enough ContentSize.\n", this);
			return -1;
		}
	}

	memcpy(&fHttpContent[fHttpContentLen], inData, inDataLen);
	fHttpContentLen += inDataLen;
	memcpy(&fHttpContent[fHttpContentLen], "\r\n\r\n", 4);
	fHttpContentLen += 4;

	return 0;
}


SInt64 HttpSession::Run()
{
	EventFlags theEvents = this->GetEvents();

	if ((theEvents & Task::kTimeoutEvent) || (theEvents & Task::kKillEvent))
    {
    	XServer::GetServer()->WriteLog("HttpSession[%p]: Timeout.", this);
    	fLiveSession = false;
    }

	if(theEvents & Task::kStartEvent)
    {
    	if (this->ConnectToServer() == 0)
        	return kIdleTimeInMilSec;

    	this->Cleanup();
    	return -1;
    }

	if (theEvents & Task::kReadEvent)
    {
    	int err = 0;
    	while (err != EAGAIN)
        {
        	if (fRecvBufferLen == kBufferSize)
            {
            	this->Signal(Task::kReadEvent);
            	break;
            }
            
        	UInt32 theLen = 0;
        	err = fSocket.Read(&fRecvBuffer[fRecvBufferLen], kBufferSize - fRecvBufferLen, &theLen);
        	fRecvBufferLen += theLen;     

        	if (err != 0 && err != EAGAIN)
            {
            	XServer::GetServer()->WriteLog("HttpSession[%p]: Read Socket Closed Error %d.", this, err);
				fState = kCleanup;
				break;
            }
        }
    }

	return this->DoTranscation();
}

SInt64 HttpSession::DoTranscation() 
{
	while (this->IsLiveSession())
    {
    	switch (fState)
        {    
        	case kSendingHeader:
			{
                if (fHttpHeaderLen > 0)
                {  
                    UInt32 theLen = 0;
                    int theErr = fSocket.Send(&fHttpHeader[fHttpHeaderStart], fHttpHeaderLen, &theLen);
                    fHttpHeaderStart += theLen;
                    fHttpHeaderLen -= theLen;

					//printf("HttpSession[%p]: Send Header %u\n", this, fHttpHeaderStart);
                    if (theErr != 0 && theErr != EAGAIN)
                    {
						XServer::GetServer()->WriteLog("HttpSession[%p]: Send Header Error %s.", this, ::strerror(errno));
                    	fState = kCleanup;
                    	break;
                    }
                }

                if (fHttpHeaderLen == 0)
                {
                	//printf("HttpPostSession[%p]: Send Header Successful %u Bytes\n", this, fHttpHeaderStart);
                    fState = kSendingContent;
                   	break;
                }
            
                fState = kSendingHeader;
                return kIdleTimeInMilSec;
			}
        	case kSendingContent:
            {                  
                if (fHttpContentLen > 0)
                {                   
                    UInt32 theLen = 0;
                    int theErr = fSocket.Send(&fHttpContent[fHttpContentStart], fHttpContentLen, &theLen);
                    fHttpContentStart += theLen;
                    fHttpContentLen -= theLen;

					//printf("HttpSession[%p]: Send Content %u\n", this, fHttpContentStart);
                    if (theErr != 0 && theErr != EAGAIN)
                    {
						XServer::GetServer()->WriteLog("HttpSession[%p]: Send Error %s.", this, ::strerror(errno));
                    	fState = kCleanup;
                    	break;
                    }
                }

                if (fHttpContentLen == 0)
                {
                	//printf("HttpPostSession[%p]: Send Content Successful %u Bytes\n", this, fHttpContentStart);
                    fState = kSendingEnd;
                    break;
                }
            
                fState = kSendingContent;
                return kIdleTimeInMilSec;
            }
			case kSendingEnd:
            {                  
                if (fHttpEndLen > 0)
                {                   
                    UInt32 theLen = 0;
                    int theErr = fSocket.Send(&fHttpEnd[fHttpEndStart], fHttpEndLen, &theLen);
                    fHttpEndStart += theLen;
                    fHttpEndLen -= theLen;

					//printf("HttpSession[%p]: Send End %u\n", this, fHttpEndStart);
                    if (theErr != 0 && theErr != EAGAIN)
                    {
						XServer::GetServer()->WriteLog("HttpSession[%p]: Send Error %s.", this, ::strerror(errno));
                    	fState = kCleanup;
                    	break;
                    }
                }

                if (fHttpEndLen == 0)
                {
                	//printf("HttpPostSession[%p]: Send End Successful %u Bytes\n", this, fHttpContentStart);
                    fState = kPocessHeader;
                    break;
                }
            
                fState = kSendingEnd;
                return kIdleTimeInMilSec;
            }
			case kPocessHeader:
			{
				fState = kPocessHeader;
				if (fRecvBufferLen == 0)
					return kIdleTimeInMilSec;

				char* theHeaderEnd = strstr(fRecvBuffer, "\r\n\r\n");
				if (theHeaderEnd == NULL)
					return kIdleTimeInMilSec;

				
				fRecvBuffer[fRecvBufferLen] = 0;
				sscanf(fRecvBuffer, "HTTP/1.1 %d", &fHttpCode);
			
				fContent = theHeaderEnd + 4;
				sscanf(fRecvBuffer, "Content-Length: %u", &fContentLength);
				UInt32 theTotalLen = fContentLength + (UInt32)(fContent - fRecvBuffer);
				
				if (fRecvBufferLen < theTotalLen)
					return kIdleTimeInMilSec; 
				
				fRecvBuffer[fRecvBufferLen] = 0;
				fState = kProcessRespond;
				break;
			}
            case kProcessRespond:
            {
               this->ProcessRespond();
			   fState = kCleanup;
			   break;
            }
        	case kCleanup:
            {
            	this->Cleanup();
            	return -1;
            }
        }
    }

	this->Cleanup();
	return -1;
}


int HttpSession::ConnectToServer()
{
	int err = fSocket.Open();
	if (err == -1)
    {
    	XServer::GetServer()->WriteLog("HttpSession[%p]:  Open Socket Error %s.", this, ::strerror(errno));
    	return -1;
    }
    
	fSocket.NoDelay();
	fSocket.KeepAlive();
	fSocket.SetTask(this);
	fSocket.SetSocketSndBufSize(96L * 1024L);

    XServer* theServer = XServer::GetServer();

    char *theWebIP = theServer->GetWebIP();
	UInt32 theRemoteAddr = SocketUtils::ConvertStringToAddr(theWebIP);
	UInt16 thePort = theServer->GetWebPort();
	err = fSocket.Connect(theRemoteAddr, thePort);
	if (err != 0 && err != EINPROGRESS)
    {
    	XServer::GetServer()->WriteLog("HttpSession[%p]: Connect Error %s.", this, ::strerror(errno));
    	return -1;
    }

	fSocket.RequestEvent(EV_RE);
	return 0;
}

