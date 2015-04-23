
#ifndef kVersionString
#include "revision.h"
#endif

#include "Log.h"

static bool sLogTimeInGMT = false;

static const char* sLogHeader = "#Software: %s\n"
                                "#Version: %s\n"    //%s == version
                                "#Date: %s\n"       //%s == date/time
                                "#Remark: All date values are in %s.\n" //%s == "GMT" or "local time"
                                "#Fields: date time filepath title copyright comment author artist album duration result\n";

Log::Log(const char* defaultPathStr, const char* logNameStr, bool enabled) 
 :  RollingLog() 
{
    this->SetTaskName("Log");
    *fDirPath = 0;
    *fLogFileName = 0;
    fWantsLogging = false;

    if (enabled)
    {
        fWantsLogging = true;
        //::strcpy(fDirPath, defaultPathStr);

		/*
        char* nameBegins = ::strrchr(fDirPath, kPathDelimiterChar);
        if ( nameBegins )
        {
            *nameBegins = 0; // terminate fDirPath at the last PathDelimeter
            nameBegins++;
            ::strcpy(fLogFileName, nameBegins);
        }
        else
        {   // it was just a file name, no dir spec'd
            ::strcpy(fDirPath, defaultPathStr);
            ::strcpy(fLogFileName, logNameStr);
        }
		*/

		::strcpy(fDirPath, defaultPathStr);
		::strcpy(fLogFileName, logNameStr);
    }

    this->SetLoggingEnabled(true);
}

time_t Log::WriteLogHeader(FILE *inFile)
{
    // Write a W3C compatable log header
    time_t calendarTime = ::time(NULL);
    Assert(-1 != calendarTime);
    if (-1 == calendarTime)
        return -1;

    struct tm  timeResult;
    struct tm* theLocalTime = ::localtime_r(&calendarTime, &timeResult);
    Assert(NULL != theLocalTime);
    if (NULL == theLocalTime)
        return -1;
     
    char tempBuffer[1024] = { 0 };
    ::strftime(tempBuffer, sizeof(tempBuffer), "#Log File Created On: %m/%d/%Y %H:%M:%S\n", theLocalTime);
    this->WriteToLog(tempBuffer, !kAllowLogToRoll);
    tempBuffer[0] = '\0';
    
    // format a date for the startup time
    
    char theDateBuffer[kMaxDateBufferSizeInBytes] = { 0 };
    Bool16 result = FormatDate(theDateBuffer, false);
    
    if (result)
    {
        ::sprintf(tempBuffer, sLogHeader, "Access Server" , kVersionString, 
                            theDateBuffer, sLogTimeInGMT ? "GMT" : "local time");
        this->WriteToLog(tempBuffer, !kAllowLogToRoll);
    }
        
    return calendarTime;
}

void Log::LogInfo(const char* infoStr)
{
    // log a generic comment 
    char    strBuff[kLogDataSize] = "";
    char    dateBuff[80] = "";
    
    if (this->FormatDate(dateBuff, false))
    {   
        if ((NULL != infoStr) && ((::strlen(infoStr) + ::strlen(strBuff) + ::strlen(dateBuff)) < 800))
        {
            ::sprintf(strBuff, "#Remark: %s %s\n", dateBuff, infoStr);
            this->WriteToLog(strBuff, kAllowLogToRoll);
        }
        else
        {   
            ::strcat(strBuff, dateBuff);
            ::strcat(strBuff," internal error in LogInfo\n");
            this->WriteToLog(strBuff, kAllowLogToRoll);       
        }
    }
}

