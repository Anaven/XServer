#ifndef _OS_H_
#define _OS_H_

#include <string.h>

#include "Headers.h"
#include "Mutex.h"

class OS
{
    public:
    
        //call this before calling anything else
        static void Initialize();

        static SInt32 Min(SInt32 a, SInt32 b)   { if (a < b) return a; return b; }
        
        //
        // Milliseconds always returns milliseconds since Jan 1, 1970 GMT.
        // This basically makes it the same as a POSIX time_t value, except
        // in msec, not seconds. To convert to a time_t, divide by 1000.
        static SInt64	Milliseconds();
        static SInt64	Microseconds();
        
        // Some processors (MIPS, Sparc) cannot handle non word aligned memory
        // accesses. So, we need to provide functions to safely get at non-word
        // aligned memory.
        static inline UInt32    Getuint32_tFromMemory(UInt32* inP);

        //because the OS doesn't seem to have these functions
        static SInt64	HostToNetworkint64_t(SInt64 hostOrdered);
        static SInt64	NetworkToHostint64_t(SInt64 networkOrdered);
                            
    	static SInt64	TimeMilli_To_Fixed64Secs(SInt64 inMilliseconds); //new CISCO provided implementation
        //disable: calculates integer value only                { return (SInt64) ( (double) inMilliseconds / 1000) * ((SInt64) 1 << 32 ) ; }
    	static SInt64	Fixed64Secs_To_TimeMilli(SInt64 inFixed64Secs)
        { UInt64 value = (UInt64) inFixed64Secs; return (value >> 32) * 1000 + (((value % ((UInt64) 1 << 32)) * 1000) >> 32); }
        
        //This converts the local time (from OS::Milliseconds) to NTP time.
    	static SInt64	TimeMilli_To_1900Fixed64Secs(SInt64 inMilliseconds)
        { return TimeMilli_To_Fixed64Secs(sMsecSince1900) + TimeMilli_To_Fixed64Secs(inMilliseconds); }

    	static SInt64	TimeMilli_To_UnixTimeMilli(SInt64 inMilliseconds) { return inMilliseconds; }

    	static time_t	TimeMilli_To_UnixTimeSecs(SInt64 inMilliseconds)
        { return (time_t)  ((SInt64) TimeMilli_To_UnixTimeMilli(inMilliseconds) / (SInt64) 1000); }
        
    	static time_t	UnixTime_Secs(void) // Seconds since 1970
        { return TimeMilli_To_UnixTimeSecs(Milliseconds()); }

        static time_t	Time1900Fixed64Secs_To_UnixTimeSecs(SInt64 in1900Fixed64Secs)
        { return (time_t)((SInt64)((SInt64)( in1900Fixed64Secs - TimeMilli_To_Fixed64Secs(sMsecSince1900)) /  ((SInt64) 1 << 32))); }
                            
        static SInt64	Time1900Fixed64Secs_To_TimeMilli(SInt64 in1900Fixed64Secs)
        { return ((SInt64)((double)((SInt64)in1900Fixed64Secs - (SInt64)TimeMilli_To_Fixed64Secs(sMsecSince1900)) / (double)((SInt64) 1 << 32)) * 1000) ; }
 
        // Returns the offset in hours between local time and GMT (or UTC) time.
        static int	GetGMTOffset();

		static bool FormatDate(char *ioDateBuffer, bool logTimeInGMT);

        //Both these functions return QTSS_NoErr, QTSS_FileExists, or POSIX errorcode
        //Makes whatever directories in this path that don't exist yet 
    	static int RecursiveMakeDir(char *inPath);
        //Makes the directory at the end of this path
    	static int MakeDir(char *inPath);

        // Discovery of how many processors are on this machine
        static UInt32	GetNumProcessors();
        
        // CPU Load
        static float  	GetCurrentCPULoadPercent();
      
        static SInt64  	InitialMSec()           { return sInitialMsec; }
        static float 	StartTimeMilli_Float()    { return (float)((double)((SInt64)OS::Milliseconds() - (SInt64) OS::InitialMSec()) / (double) 1000.0); }
        static SInt64  	StartTimeMilli_Int()     { return (OS::Milliseconds() - OS::InitialMSec()); }

    	static bool 	ThreadSafe();

    	static SInt64 	StrfmtToTime(char *inTimeStr);

   private:
       
        static double sDivisor;
        static double sMicroDivisor;
        static SInt64 sMsecSince1900;
        static SInt64 sMsecSince1970;
        static SInt64 sInitialMsec;
        static SInt32 sMemoryErr;
        static SInt64 sWrapTime;
        static SInt64 sCompareWrap;
        static SInt64 sLastTimeMilli;
};

inline UInt32   OS::Getuint32_tFromMemory(UInt32* inP)
{
#if ALLOW_NON_WORD_ALIGN_ACCESS
    return *inP;
#else
    char* tempPtr = (char*)inP;
    UInt32 temp = 0;
    ::memcpy(&temp, tempPtr, sizeof(UInt32));
    return temp;
#endif
}


#endif
