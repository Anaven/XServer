#include "XServer.h"
#include "EnrollReportSession.h"

void EnrollReportSession::ConstructURL(char* theSeverID, char* theAuthName, char *theDeviceID, int theState,
									char* theWebIP, UInt16 theWebPort)
{
    if (theState == kDeviceLogin) // device login or change state
    {                                                        
        snprintf(fURL, kURLSize, "/WebService/RegisterDeviceToService.aspx?sid=%s&did=%s&state=%d", \
                              	theSeverID, theDeviceID, theState);
    }
    else if (theState == kDeviceLogout)// device logout
    { 
        snprintf(fURL, kURLSize, "/WebService/RemoveDevice.aspx?id=%s", theDeviceID);
    }
	else if (theState == kUserLogin)
    {
        snprintf(fURL, kURLSize, "/WebService/SetDeviceState.aspx?uid=%s&did=%s&state=1", \
                            	theAuthName, theDeviceID);
    }
	else if (theState == kUserLogout)
    {
        snprintf(fURL, kURLSize, "/WebService/SetDeviceState.aspx?uid=%s&did=%s&state=0", \
                              	theAuthName, theDeviceID);
    }
    else if (theState == kObserverLogin)
    {
        snprintf(fURL, kURLSize, "/WebService/UserWatchDevice.aspx?uid=%s&did=%s&mode=1", \
                             	theAuthName, theDeviceID);
    }
    else if (theState == kObserverLogout)
    {
        snprintf(fURL, kURLSize, "/WebService/UserWatchDevice.aspx?uid=%s&did=%s&mode=0", \
                           		theAuthName, theDeviceID);
    }

	XServer::GetServer()->WriteLog("EnrollReportSession[%p]: %s.", this, fURL);
	this->ConstructHeader(fURL, theWebIP, theWebPort);
}

int EnrollReportSession::ProcessRespond()
{
	if (fHttpCode != 200) // i == 2 is the index of http 200
	{
		XServer::GetServer()->WriteLog("EnrollReportSession[%p]: HTTP/1.1 %d.", this, fHttpCode);
		return kHttpFailed;
	}


#ifdef ENROLL_REPORT_SESSION_DEBUG
	XServer::GetServer()->WriteLog("EnrollReportSession[%p]: %s", this, fContent);
#endif

	char* errorCodeStr = strstr(fContent, "ErrorCode:");
	if (errorCodeStr == NULL)
	{
		XServer::GetServer()->WriteLog("EnrollReportSession[%p]: No ErrorCode.", this);
		return kHttpFailed;
	}

    return kHttpSuccess;
}


