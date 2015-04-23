#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <sys/time.h>

#include "OS.h"
#include "Thread.h"
#include "MyAssert.h"

double  OS::sDivisor = 0;
double  OS::sMicroDivisor = 0;
SInt64  OS::sMsecSince1970 = 0;
SInt64  OS::sMsecSince1900 = 0;
SInt64  OS::sInitialMsec = 0;
SInt64  OS::sWrapTime = 0;
SInt64  OS::sCompareWrap = 0;
SInt64  OS::sLastTimeMilli = 0;

//#include "Mutex.h"

//static Mutex* sLastMillisMutex = NULL;

void OS::Initialize()
{
    Assert (sInitialMsec == 0);  // do only once
    if (sInitialMsec != 0) return;
    ::tzset();

    //setup t0 value for msec since 1900
    //t.tv_sec is number of seconds since Jan 1, 1970. Convert to seconds since 1900    
    SInt64 the1900Sec = (SInt64) (24 * 60 * 60) * (SInt64) ((70 * 365) + 17) ;
    sMsecSince1900 = the1900Sec * 1000;
    
    sWrapTime = (SInt64) 0x00000001 << 32;
    sCompareWrap = (SInt64) 0xffffffff << 32;
    sLastTimeMilli = 0;
    
    sInitialMsec = OS::Milliseconds(); //Milliseconds uses sInitialMsec so this assignment is valid only once.

    sMsecSince1970 = ::time(NULL);  // POSIX time always returns seconds since 1970
    sMsecSince1970 *= 1000;         // Convert to msec

    //sLastMillisMutex = new Mutex();
}

SInt64 OS::Milliseconds()
{
    //MutexLocker locker(sLastMillisMutex);

    struct timeval t;
    int theErr = ::gettimeofday(&t, NULL);
    Assert(theErr == 0);

    SInt64 curTime;
    curTime = t.tv_sec;
    curTime *= 1000;                // sec -> msec
    curTime += t.tv_usec / 1000;    // usec -> msec

    return (curTime - sInitialMsec) + sMsecSince1970;
}

SInt64 OS::Microseconds()
{
    struct timeval t;
    int theErr = ::gettimeofday(&t, NULL);
    Assert(theErr == 0);

    SInt64 curTime;
    curTime = t.tv_sec;
    curTime *= 1000000;     // sec -> usec
    curTime += t.tv_usec;

    return curTime - (sInitialMsec * 1000);
}

int OS::GetGMTOffset()
{    
    time_t clock;
    struct tm  *tmptr= localtime(&clock);
    if (tmptr == NULL)
        return 0;
        
    return tmptr->tm_gmtoff / 3600;//convert seconds to  hours before or after GMT
}


SInt64  OS::HostToNetworkint64_t(SInt64 hostOrdered)
{
#if BIGENDIAN
    return hostOrdered;
#else
    return (SInt64) (  (UInt64)  (hostOrdered << 56) | (UInt64)  (((UInt64) 0x00ff0000 << 32) & (hostOrdered << 40))
        | (UInt64)  ( ((UInt64)  0x0000ff00 << 32) & (hostOrdered << 24)) | (UInt64)  (((UInt64)  0x000000ff << 32) & (hostOrdered << 8))
        | (UInt64)  ( ((UInt64)  0x00ff0000 << 8) & (hostOrdered >> 8)) | (UInt64)     ((UInt64)  0x00ff0000 & (hostOrdered >> 24))
        | (UInt64)  (  (UInt64)  0x0000ff00 & (hostOrdered >> 40)) | (UInt64)  ((UInt64)  0x00ff & (hostOrdered >> 56)) );
#endif
}

SInt64  OS::NetworkToHostint64_t(SInt64 networkOrdered)
{
#if BIGENDIAN
    return networkOrdered;
#else
    return (SInt64) (  (UInt64)  (networkOrdered << 56) | (UInt64)  (((UInt64) 0x00ff0000 << 32) & (networkOrdered << 40))
        | (UInt64)  ( ((UInt64)  0x0000ff00 << 32) & (networkOrdered << 24)) | (UInt64)  (((UInt64)  0x000000ff << 32) & (networkOrdered << 8))
        | (UInt64)  ( ((UInt64)  0x00ff0000 << 8) & (networkOrdered >> 8)) | (UInt64)     ((UInt64)  0x00ff0000 & (networkOrdered >> 24))
        | (UInt64)  (  (UInt64)  0x0000ff00 & (networkOrdered >> 40)) | (UInt64)  ((UInt64)  0x00ff & (networkOrdered >> 56)) );
#endif
}

int OS::RecursiveMakeDir(char *inPath)
{
    Assert(inPath != NULL);
    
    //iterate through the path, replacing '/' with '\0' as we go
    char *thePathTraverser = inPath;
    
    //skip over the first / in the path.
    if (*thePathTraverser == kPathDelimiterChar)
        thePathTraverser++;
        
    while (*thePathTraverser != '\0')
    {
        if (*thePathTraverser == kPathDelimiterChar)
        {
            //we've found a filename divider. Now that we have a complete
            //filename, see if this partial path exists.
            
            //make the partial path into a C string
            *thePathTraverser = '\0';
            int theErr = MakeDir(inPath);
            //there is a directory here. Just continue in our traversal
            *thePathTraverser = kPathDelimiterChar;

            if (theErr != 0)
                return theErr;
        }
        thePathTraverser++;
    }
    
    //need to create the last directory in the path
    return MakeDir(inPath);
}

int OS::MakeDir(char *inPath)
{
    struct stat theStatBuffer;
    if (::stat(inPath, &theStatBuffer) == -1)
    {
        //this directory doesn't exist, so let's try to create it
        if (::mkdir(inPath, S_IRWXU) == -1)
            return Thread::GetErrno();
    }
    else if (!S_ISDIR(theStatBuffer.st_mode))
        return EEXIST;//there is a file at this point in the path!

    //directory exists
    return 0;
}

bool OS::ThreadSafe()
{
	return true;
}

UInt32 OS::GetNumProcessors()
{
    UInt32 numCPUs = sysconf(_SC_NPROCESSORS_CONF); ;
    Assert(numCPUs > 0);

    return numCPUs;
}

//CISCO provided fix for integer + fractional fixed64.
SInt64 OS::TimeMilli_To_Fixed64Secs(SInt64 inMilliseconds)
{
       SInt64 result = inMilliseconds / 1000;  // The result is in lower bits.
       result <<= 32;  // shift it to higher 32 bits
       // Take the remainder (rem = inMilliseconds%1000) and multiply by
       // 2**32, divide by 1000, effectively this gives (rem/1000) as a
       // binary fraction.
       double p = ldexp((double)(inMilliseconds%1000), +32) / 1000.;
       UInt32 frac = (UInt32)p;
       result |= frac;
       return result;
}

SInt64 OS::StrfmtToTime(char *inTimeStr)
{
	struct tm tm;
    
    ::sscanf(inTimeStr, "%d/%d/%d %d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, 
                             &tm.tm_hour, &tm.tm_min, &tm.tm_sec);

	tm.tm_year  -= 1900;
	tm.tm_mon   -= 1;
	tm.tm_isdst = -1;

	SInt64 curTime = ::mktime(&tm);
	curTime *= 1000;
    
	return (curTime - sInitialMsec) + sMsecSince1970;
}

bool OS::FormatDate(char *ioDateBuffer, bool logTimeInGMT)
{
    Assert(NULL != ioDateBuffer);
    
    //use ansi routines for getting the date.
    time_t calendarTime = ::time(NULL);
    Assert(-1 != calendarTime);
    if (-1 == calendarTime)
        return false;
        
    struct tm* theTime = NULL;
    struct tm  timeResult;
    
    if (logTimeInGMT)
        theTime = ::gmtime_r(&calendarTime, &timeResult);
    else
        theTime = ::localtime_r(&calendarTime, &timeResult);
    
    Assert(NULL != theTime);
    
    if (NULL == theTime)
        return false;
        
    // date time needs to look like this for extended log file format: 2001-03-16 23:34:54
    // this wonderful ANSI routine just does it for you.
    // the format is YYYY-MM-DD HH:MM:SS
    // the date time is in GMT, unless logTimeInGMT is false, in which case
    // the time logged is local time
    //qtss_strftime(ioDateBuffer, kMaxDateBufferSize, "%d/%b/%Y:%H:%M:%S", theLocalTime);
    ::strftime(ioDateBuffer, 30, "%Y/%m/%d %H:%M:%S", theTime);  
    return true;
}

