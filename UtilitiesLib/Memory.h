/*
    File:       Memory.h

    Contains:   Prototypes for overridden new & delete, definition of Memory
                class which implements some memory leak debugging features.                  
*/

#ifndef __OS_MEMORY_H__
#define __OS_MEMORY_H__

#include "Headers.h"
#include "Queue.h"
#include "Mutex.h"

#ifndef Assert
#define Assert(x) assert(x) 
#endif

class Memory
{
    public:
    
#if MEMORY_DEBUGGING
        //If memory debugging is on, clients can get access to data structures that give
        //memory status.
        static Queue* 	GetTagQueue() { return &sTagQueue; }
        static Mutex* 	GetTagQueueMutex() { return &sMutex;    }
        static UInt32	GetAllocatedMemory() { return sAllocatedBytes; }

        static void*    DebugNew(size_t size, char* inFile, int inLine, bool sizeCheck);
        static void     DebugDelete(void *mem);
        static bool		MemoryDebuggingTest();
        static void     ValidateMemoryQueue();

        enum
        {
            kMaxFileNameSize = 48
        };
        
        struct TagElem
        {
            QueueElem 	elem;
            char     	fileName[kMaxFileNameSize];
            int     	line;
            UInt32 	tagSize; //how big are objects of this type?
            UInt32 	totMemory; //how much do they currently occupy
            UInt32 	numObjects;//how many are there currently?
        };
#endif

        // Provides non-debugging behaviour for new and delete
        static void*	New(size_t inSize);
        static void     Delete(void* inMemory);
        
        //When memory allocation fails, the server just exits. This sets the code
        //the server exits with
        static void 	SetMemoryError(SInt32 inErr);
        
#if MEMORY_DEBUGGING
    private:
    
        struct MemoryDebugging
        {
            QueueElem elem;
            TagElem* tagElem;
            UInt32 size;
        };
        static Queue 	sMemoryQueue;
        static Queue 	sTagQueue;
        static UInt32	sAllocatedBytes;
        static Mutex 	sMutex;

#endif
};


// NEW MACRO
// When memory debugging is on, this macro transparently uses the memory debugging
// overridden version of the new operator. When memory debugging is off, it just compiles
// down to the standard new.

#if MEMORY_DEBUGGING

#ifdef  NEW
#error Conflicting Macro "NEW"
#endif

#define NEW new (__FILE__, __LINE__)

#else

#ifdef  NEW
#error Conflicting Macro "NEW"
#endif

#define NEW new

#endif


// 
// PLACEMENT NEW OPERATOR
inline void* operator new(size_t, void* ptr) { return ptr;}

#if MEMORY_DEBUGGING

// These versions of the new operator with extra arguments provide memory debugging
// features.

void* operator new(size_t s, char* inFile, int inLine);
void* operator new[](size_t s, char* inFile, int inLine);


#endif

// When memory debugging is not on, these are overridden so that if new fails,
// the process will exit.

void* operator new (size_t s);
void* operator new[](size_t s);

void operator delete(void* mem);
void operator delete[](void* mem);

#endif //__OS_MEMORY_H__
