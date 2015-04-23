#ifndef __DEVICES_REPORT_SESSION__
#define __DEVICES_REPORT_SESSION__

#include "HttpSession.h"

#define DEVICES_REPORT_SESSION_DEBUG 1

class DevicesReportSession : public HttpSession
{
	public:

    	DevicesReportSession() : HttpSession(kHttpGet) {};
        ~DevicesReportSession() {};
            
    	void ConstructURL(char* theServerID, char* theWebIP, UInt16 theWebPort);
    	int ProcessRespond();
};

#endif
