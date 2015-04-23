/*
    File:       StopWatch.h

    Contains:   classes for stopwatch timers        
    
*/ 


#include "OS.h"

class MilliSecondStopWatch 
{
    public:
        
    	MilliSecondStopWatch () { fStartedAt = -1; fStoppedAt = -1; }

    	void Start() { fStartedAt = OS::Milliseconds(); }
    	void Stop() { fStoppedAt = OS::Milliseconds(); }
    	SInt64 Duration() { return fStoppedAt - fStartedAt; }
        
    private:
        
    	SInt64 fStartedAt;
    	SInt64 fStoppedAt;
};



class MicroSecondStopWatch 
{

    public:
        
    	MicroSecondStopWatch () { fStartedAt = -1; fStoppedAt = -1; }

    	void Start() { fStartedAt = OS::Microseconds(); }
    	void Stop() { fStoppedAt = OS::Microseconds(); }
    	SInt64 Duration() { return fStoppedAt - fStartedAt; }
        
    private:
        
        SInt64 fStartedAt;
        SInt64 fStoppedAt;
};

