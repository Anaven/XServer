#ifndef _MYASSERT_H_
#define _MYASSERT_H_

#include <stdio.h>

#ifdef __cplusplus
class AssertLogger
{
    public:
        // An interface so the MyAssert function can write a message
        virtual void LogAssert(char* inMessage) = 0;
    	virtual ~AssertLogger(){};
};

// If a logger is provided, asserts will be logged. Otherwise, asserts will cause a bus error
void SetAssertLogger(AssertLogger* theLogger);
#endif

#if ASSERT  
    void MyAssert(char *s);

    #define kAssertBuffSize 256
    
    #define Assert(condition)    {                              \
                                                                \
        if (!(condition))                                       \
        {                                                       \
            char s[kAssertBuffSize];                            \
            s[kAssertBuffSize -1] = 0;                          \
            snprintf(s, kAssertBuffSize -1, "_Assert: %s, %d", __FILE__, __LINE__); \
            MyAssert(s);                                        \
        }   }


    #define AssertV(condition,errNo)    {                                   \
        if (!(condition))                                                   \
        {                                                                   \
            char s[kAssertBuffSize];                                        \
            s[kAssertBuffSize -1] = 0;                                      \
            snprintf(s, kAssertBuffSize -1, "_AssertV: %s, %d (%d)", __FILE__, __LINE__, errNo);    \
            MyAssert(s);                                                    \
        }   }
                                     
                                         
    #define Warn(condition) {                                       \
            if (!(condition))                                       \
                printf("_Warn: %s, %d\n", __FILE__, __LINE__ );     }                                                           
                                         
    #define WarnV(condition,msg)        {                               \
        if (!(condition))                                               \
            printf("_WarnV: %s, %d (%s)\n", __FILE__, __LINE__, msg );  }                                                   
                                         
    #define WarnVE(condition,msg,err)  {                                   \
        if (!(condition))                                               \
        {   char buffer[kAssertBuffSize];                                \
            buffer[kAssertBuffSize -1] = 0;                              \
            printf("_WarnV: %s, %d (%s, %s [err=%d])\n", __FILE__, __LINE__, msg, qtss_strerror(err, buffer, sizeof(buffer) -1), err);  \
        }    }

#else
            
    #define Assert(condition) ((void) 0)
    #define AssertV(condition,errNo) ((void) 0)
    #define Warn(condition) ((void) 0)
    #define WarnV(condition,msg) ((void) 0)

#endif
#endif //_MY_ASSERT_H_
