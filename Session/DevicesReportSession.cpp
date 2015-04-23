#include "XServer.h"
#include "DevicesReportSession.h"
#include "MessageClient.h"
#include "MessageDevice.h"


void DevicesReportSession::ConstructURL(char* theServerID, char* theWebIP, UInt16 theWebPort)
{
    RefTable *theMessageDeviceRefTable = XServer::GetMessageDeviceRefTable();
    char tempData[64];
    int i = 0;
	
    
    MutexLocker theLocker(theMessageDeviceRefTable->GetMutex());
    RefHashTable* theDeviceHashTable = theMessageDeviceRefTable->GetHashTable();

    snprintf(fURL, kURLSize, "/WebService/RegisterDevicesToService.aspx?count=%lld&sid=%s",
							theDeviceHashTable->GetNumEntries(), theServerID);
    
    for (RefHashTableIter theDeviceIter(theDeviceHashTable); !theDeviceIter.IsDone(); theDeviceIter.Next())
    {
    	MessageDevice *theDevice = (MessageDevice*)theDeviceIter.GetCurrent()->GetObject();

        int theIsOccupied = theDevice->HasClient() ? 1 : 0;
        snprintf(tempData, 64, "&did%d=%s&state%d=%d", i, theDevice->GetDeviceID(), i, theIsOccupied);
        strcat(fURL, tempData);
        i++;
    }

	XServer::GetServer()->WriteLog("DevicesReportSession[%p]: %s", this, fURL);
	this->ConstructHeader(fURL, theWebIP, theWebPort);
}


int DevicesReportSession::ProcessRespond()
{
	if (fHttpCode != 200) // i == 2 is the index of http 200
    {
    	XServer::GetServer()->WriteLog("DevicesReportSession[%p]: HTTP/1.1 %d.", this, fHttpCode);
        return kHttpFailed;
    }


#ifdef DEVICES_REPORT_SESSION_DEBUG
	XServer::GetServer()->WriteLog("DevicesReportSession[%p]: %s", this, fContent);
#endif

    char* errorCodeStr = strstr(fContent, "ErrorCode:");
	if (errorCodeStr == NULL)
    {
    	XServer::GetServer()->WriteLog("DevicesReportSession[%p]: No ErrorCode.", this);
        return kHttpFailed;
    }

    return kHttpSuccess;
}


