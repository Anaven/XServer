#include "MyAssert.h"

static AssertLogger* sLogger = NULL;

void SetAssertLogger(AssertLogger* theLogger)
{
    sLogger = theLogger;
}

void MyAssert(char *inMessage)
{
    if (sLogger != NULL)
        sLogger->LogAssert(inMessage);
    else
    {
    	printf("%s\n", inMessage);
        (*(int*)0) = 0;
    }
}
