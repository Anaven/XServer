#include "XServer.h"
#include "ServerReportSession.h"

void ServerReportSession::ConstructURL(char* theServerID, char* theServerIP, UInt16 theMessagePort, 
                                    UInt16 theAudioPort, UInt16 theVideoPort, char* theWebIP, UInt16 theWebPort)
{
    snprintf(fURL, kURLSize, "/WebService/RegisterService.aspx?id=%s&ip=%s&mp=%d&ap=%d&vp=%d", \
                         	theServerID, theServerIP, theMessagePort, theAudioPort, theVideoPort);

	XServer::GetServer()->WriteLog("ServerReportSession[%p]: %s", this, fURL);
    this->ConstructHeader(fURL, theWebIP, theWebPort);
}

int ServerReportSession::ProcessRespond()
{
    if (fHttpCode != 200) // i == 2 is the index of http 200
    {
    	XServer::GetServer()->WriteLog("ServerReportSession[%p]: HTTP/1.1 %d.", this, fHttpCode);
        return kHttpFailed;
    }

#ifdef SERVER_REPORT_SESSION_DEBUG
	XServer::GetServer()->WriteLog("ServerReportSession[%p]: %s", this, fContent);
#endif

	char* errorCodeStr = strstr(fContent, "ErrorCode:");
	if (errorCodeStr == NULL)
	{
		XServer::GetServer()->WriteLog("ServerReportSession[%p]: No ErrorCode.", this);
		return kHttpFailed;
	}

	return kHttpSuccess;
}

