#ifndef __SERVER_REPORT_SESSION__
#define __SERVER_REPORT_SESSION__

#include "HttpSession.h"

#define SERVER_REPORT_SESSION_DEBUG 1

class ServerReportSession : public HttpSession
{
	public:

    	ServerReportSession() : HttpSession(kHttpGet) {};
        ~ServerReportSession() {};
            
    	void ConstructURL(char* theServerID, char* theServerIP, UInt16 theMessagePort, 
						UInt16 theAudioPort, UInt16 theVideoPort, char* theWebIP, 
						UInt16 theWebPort);
    	int ProcessRespond();
};

#endif
