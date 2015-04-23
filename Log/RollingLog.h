/*
    File:       RollingLog.h

    Contains:   A log toolkit, log can roll either by time or by size, clients
                must derive off of this object ot provide configuration information. 
*/

#ifndef __QTSS_ROLLINGLOG_H__
#define __QTSS_ROLLINGLOG_H__

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "Mutex.h"
#include "Task.h"

const bool kAllowLogToRoll = true;

class RollingLog : public Task
{
    public:
    
        //pass in whether you'd like the log roller to log errors.
        RollingLog();
        
        //
        // Call this to delete. Closes the log and sends a kill event
        void Delete() { CloseLog(false); this->Signal(Task::kKillEvent); }
        
        //
        // Write a log message
        void WriteToLog(char* inLogData, bool allowLogToRoll);
        
        //log rolls automatically based on the configuration criteria,
        //but you may roll the log manually by calling this function.
        //Returns true if no error, false otherwise
        bool RollLog();

        //
        // Call this to open the log file and begin logging     
        void EnableLog( bool appendDotLog = true);
        
                //
        // Call this to close the log
        // (pass leaveEnabled as true when we are temporarily closing.)
        void CloseLog( bool leaveEnabled = false);

        //
        //mainly to check and see if errors occurred
        bool IsLogEnabled();
        
        //master switch
        bool IsLogging() { return fLogging; }
        void SetLoggingEnabled( bool logState ) { fLogging = logState; }
        
        //General purpose utility function
        //returns false if some error has occurred
        static bool FormatDate(char *ioDateBuffer, bool logTimeInGMT);
        
        // Check the log to see if it needs to roll
        // (rolls the log if necessary)
        bool CheckRollLog();
        
        // Set this to true to get the log to close the file between writes.
        static void SetCloseOnWrite(bool closeOnWrite);

        enum
        {
            kMaxDateBufferSizeInBytes = 30, //UInt32
            kMaxFilenameLengthInBytes = 31  //UInt32
        };
    
    protected:

        //
        // Task object. Do not delete directly
        virtual ~RollingLog();

        //Derived class must provide a way to get the log & rolled log name
        virtual char* 	GetLogName() = 0;
        virtual char* 	GetLogDir() = 0;
        virtual UInt32 	GetRollIntervalInDays() = 0;//0 means no interval
        virtual UInt32 	GetMaxLogBytes() = 0;//0 means unlimited
                    
        //to record the time the file was created (for time based rolling)
        virtual time_t  WriteLogHeader(FILE *inFile);
        time_t          ReadLogHeader(FILE* inFile);

    private:
    
        //
        // Run function to roll log right at midnight   
        virtual SInt64      Run();

        FILE*           fLog;
        time_t          fLogCreateTime;
        char*           fLogFullPath;
        bool          	fAppendDotLog;
        bool          	fLogging;
        bool          	RenameLogFile(const char* inFileName);
        bool          	DoesFileExist(const char *inPath);
        static void     ResetToMidnight(time_t* inTimePtr, time_t* outTimePtr);
        char*           GetLogPath(const char *extension);
        
        // To make sure what happens in Run doesn't also happen at the same time
        // in the public functions.
        Mutex         fMutex;
};

#endif // __QTSS_ROLLINGLOG_H__

