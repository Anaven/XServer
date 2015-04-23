#ifndef __LOG_H__
#define __LOG_H__

#include "RollingLog.h"

enum
{
	kLogDataSize = 5120,
};

class Log : public RollingLog
{
	enum { eLogMaxBytes = 0, eLogMaxDays = 1 };

    public:
        
        Log(const char* defaultPathStr, const char* logNameStr, bool enabled);
        virtual ~Log() {}

        virtual char* GetLogName()     { return fLogFileName;     }  
    	virtual char* GetLogDir()     { return fDirPath;         }  

    	virtual UInt32 GetRollIntervalInDays() { return eLogMaxDays; /* we dont' roll*/ }                                  
        virtual UInt32 GetMaxLogBytes() {  return eLogMaxBytes; /* we dont' roll*/ }

    	void    LogInfo( const char* infoStr);

    	bool	WantsLogging() { return fWantsLogging; }
    	const char* LogFileName() { return fLogFileName; }
        const char* LogDirName() { return fDirPath; }

    	virtual time_t  WriteLogHeader(FILE *inFile);

	protected:
        
        char    fDirPath[256];
        char    fLogFileName[256];
        bool    fWantsLogging;
};

#endif // __LOG_H__

