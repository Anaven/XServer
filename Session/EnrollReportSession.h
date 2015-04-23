#ifndef __ENROLL_REPORT_SESSION__
#define __ENROLL_REPORT_SESSION__

#include "HttpSession.h"

#define ENROLL_REPORT_SESSION_DEBUG 1

enum
{
	kDeviceLogin    	= 0,
	kDeviceLogout	 	= 1,
	kUserLogin			= 2,
	kUserLogout 		= 3,
	kObserverLogin		= 4,
	kObserverLogout		= 5
};

class EnrollReportSession : public HttpSession
{
	public:

    	EnrollReportSession() : HttpSession(kHttpGet) {};
        ~EnrollReportSession() {};
            
    	void ConstructURL(char* theSeverID, char* theAuthName, char *theDeviceID, int theState,
						char* theWebIP, UInt16 theWebPort);
    	int ProcessRespond();
};

#endif
