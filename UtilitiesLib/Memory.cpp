/*
    File:       Memory.cpp

    Contains:   Implementation of Memory stuff, including all new & delete
                operators.                
*/

#include <string.h>
#include "Memory.h"

#if MEMORY_DEBUGGING

Queue   Memory::sMemoryQueue;
Queue   Memory::sTagQueue;
UInt32  Memory::sAllocatedBytes = 0;
Mutex   Memory::sMutex;

#endif

static SInt32   sMemoryErr = 0;

//
// OPERATORS

#if MEMORY_DEBUGGING
void* operator new(size_t s, char* inFile, int inLine)
{
    return Memory::DebugNew(s, inFile, inLine, true);
}


void* operator new[](size_t s, char* inFile, int inLine)
{
    return Memory::DebugNew(s, inFile, inLine, false);
}
#endif

void* operator new (size_t s)
{
    return Memory::New(s);
}


void* operator new[](size_t s)
{
    return Memory::New(s);
}

void operator delete(void* mem)
{
    Memory::Delete(mem);
}


void operator delete[](void* mem)
{
    Memory::Delete(mem);
}


void Memory::SetMemoryError(SInt32 inErr)
{
    sMemoryErr = inErr;
}


void*   Memory::New(size_t inSize)
{
#if MEMORY_DEBUGGING
    return Memory::DebugNew(inSize, __FILE__, __LINE__, false);
#else
    void *m = malloc(inSize);
    if (m == NULL)
        ::exit(sMemoryErr);
    return m;
#endif
}


void    Memory::Delete(void* inMemory)
{
    if (inMemory == NULL)
        return;
#if MEMORY_DEBUGGING
    Memory::DebugDelete(inMemory);
#else
    free(inMemory);
#endif
}


#if MEMORY_DEBUGGING
void* Memory::DebugNew(size_t s, char* inFile, int inLine, bool sizeCheck)
{
    //also allocate enough space for a Q elem and a long to store the length of this
    //allocation block
    MutexLocker locker(&sMutex);
    ValidateMemoryQueue();
    UInt32 actualSize = s + sizeof(MemoryDebugging) + (2 * sizeof(inLine));
    char *m = (char *)malloc(actualSize);
    if (m == NULL)
        ::exit(sMemoryErr);

    char theFileName[kMaxFileNameSize];
    strncpy(theFileName, inFile, kMaxFileNameSize);
    theFileName[kMaxFileNameSize] = '\0';
    
    //mark the beginning and the end with the line number
    memset(m, 0xfe, actualSize);//mark the block with an easily identifiable pattern
    ::memcpy(m, &inLine, sizeof(inLine));
    ::memcpy((m + actualSize) - sizeof(inLine), &inLine, sizeof(inLine));

    TagElem* theElem = NULL;
    
    //also update the tag queue
    for (QueueIter iter(&sTagQueue); !iter.IsDone(); iter.Next())
    {
        TagElem* elem = (TagElem*)iter.GetCurrent()->GetEnclosingObject();
        if ((::strcmp(elem->fileName, theFileName) == 0) && (elem->line == inLine))
        {
            //verify that the size of this allocation is the same as all others
            //(if requested... some tags are of variable size)
            if (sizeCheck)
                Assert(s == elem->tagSize);
            elem->totMemory += s;
            elem->numObjects++;
            theElem = elem;
        }
    }
    
    if (theElem == NULL)
    {
        //if we've gotten here, this tag doesn't exist, so let's add it.
        theElem = (TagElem*)malloc(sizeof(TagElem));
        if (theElem == NULL)
            ::exit(sMemoryErr);
        memset(theElem, 0, sizeof(TagElem));
        theElem->elem.SetEnclosingObject(theElem);
        ::strcpy(theElem->fileName, theFileName);
        theElem->line = inLine;
        theElem->tagSize = s;
        theElem->totMemory = s;
        theElem->numObjects = 1;
        sTagQueue.EnQueue(&theElem->elem);
    }
    
    //put this chunk on the global chunk queue
    MemoryDebugging* header = (MemoryDebugging*)(m + sizeof(inLine));
    memset(header, 0, sizeof(MemoryDebugging));
    header->size = s;
    header->tagElem = theElem;
    header->elem.SetEnclosingObject(header);
    sMemoryQueue.EnQueue(&header->elem);
    sAllocatedBytes += s;
        
    return m + sizeof(inLine) + sizeof(MemoryDebugging);
}


void Memory::DebugDelete(void *mem)
{
    MutexLocker locker(&sMutex);
    ValidateMemoryQueue();
    char* memPtr = (char*)mem;
    MemoryDebugging* memInfo = (MemoryDebugging*)mem;
    memInfo--;//get a pointer to the MemoryDebugging structure
    Assert(memInfo->elem.IsMemberOfAnyQueue());//must be on the memory Queue
    //double check it's on the memory queue
    bool found  = false;
    for (QueueIter iter(&sMemoryQueue); !iter.IsDone(); iter.Next())
    {
        MemoryDebugging* check = (MemoryDebugging*)iter.GetCurrent()->GetEnclosingObject();
        if (check == memInfo)
        {
            found = true;
            break;
        }
    }
    Assert(found == true);
    sMemoryQueue.Remove(&memInfo->elem);
    Assert(!memInfo->elem.IsMemberOfAnyQueue());
    sAllocatedBytes -= memInfo->size;
    
    //verify that the tags placed at the very beginning and very end of the
    //block still exist
    memPtr += memInfo->size;
    int* linePtr = (int*)memPtr;
    Assert(*linePtr == memInfo->tagElem->line);
    memPtr -= sizeof(MemoryDebugging) + sizeof(int) + memInfo->size;
    linePtr = (int*)memPtr;
    Assert(*linePtr == memInfo->tagElem->line);
    
    //also update the tag queue
    Assert(memInfo->tagElem->numObjects > 0);
    memInfo->tagElem->numObjects--;
    memInfo->tagElem->totMemory -= memInfo->size;
    
    if (memInfo->tagElem->numObjects == 0)
    {
        // If this tag has no elements, then delete the tag
        Assert(memInfo->tagElem->totMemory == 0);
        sTagQueue.Remove(&memInfo->tagElem->elem);
        free(memInfo->tagElem);
    }
    
    // delete our memory block
    memset(mem, 0xfd,memInfo->size);
    free(memPtr);
}


void Memory::ValidateMemoryQueue()
{
    MutexLocker locker(&sMutex);
    for(QueueIter iter(&sMemoryQueue); !iter.IsDone(); iter.Next())
    {
        MemoryDebugging* elem = (MemoryDebugging*)iter.GetCurrent()->GetEnclosingObject();
        char* rawmem = (char*)elem;
        rawmem -= sizeof(int);
        int* tagPtr = (int*)rawmem;
        Assert(*tagPtr == elem->tagElem->line);
        rawmem += sizeof(int) + sizeof(MemoryDebugging) + elem->size;
        tagPtr = (int*)rawmem;
        Assert(*tagPtr == elem->tagElem->line);
    }
}


#if 0
bool Memory::MemoryDebuggingTest()
{
    static char* s20 = "this is 20 characte";
    static char* s30 = "this is 30 characters long, o";
    static char* s40 = "this is 40 characters long, okey dokeys";
    
    void* victim = DebugNew(20, 'tsta', true);
    strcpy((char*)victim, s20);
    MemoryDebugging* victimInfo = (MemoryDebugging*)victim;
    ValidateMemoryQueue();
    victimInfo--;
    if (victimInfo->tag != 'tsta')
        return false;
    if (victimInfo->size != 20)
        return false;
        
    void* victim2 = DebugNew(30, 'tstb', true);
    strcpy((char*)victim2, s30);
    ValidateMemoryQueue();
    void* victim3 = DebugNew(20, 'tsta', true);
    strcpy((char*)victim3, s20);
    ValidateMemoryQueue();
    void* victim4 = DebugNew(40, 'tstc', true);
    strcpy((char*)victim4, s40);
    ValidateMemoryQueue();
    void* victim5 = DebugNew(30, 'tstb', true);
    strcpy((char*)victim5, s30);
    ValidateMemoryQueue();
    
    if (sTagQueue.GetLength() != 3)
        return false;
    for (QueueIter iter(&sTagQueue); !iter.IsDone(); iter.Next())
    {
        TagElem* elem = (TagElem*)iter.GetCurrent()->GetEnclosingObject();
        if (*elem->tagPtr == 'tstb')
        {
            if (elem->tagSize != 30)
                return false;
            if (elem->numObjects != 2)
                return false;
        }
        else if (*elem->tagPtr == 'tsta')
        {
            if (elem->tagSize != 20)
                return false;
            if (elem->numObjects != 2)
                return false;
        }
        else if (*elem->tagPtr == 'tstc')
        {
            if (elem->tagSize != 40)
                return false;
            if (elem->numObjects != 1)
                return false;
        }
        else
            return false;
    }
    
    DebugDelete(victim3);
    ValidateMemoryQueue();
    DebugDelete(victim4);
    ValidateMemoryQueue();

    if (sTagQueue.GetLength() != 3)
        return false;
    for (QueueIter iter2(&sTagQueue); !iter2.IsDone(); iter2.Next())
    {
        TagElem* elem = (TagElem*)iter2.GetCurrent()->GetEnclosingObject();
        if (*elem->tagPtr == 'tstb')
        {
            if (elem->tagSize != 30)
                return false;
            if (elem->numObjects != 2)
                return false;
        }
        else if (*elem->tagPtr == 'tsta')
        {
            if (elem->tagSize != 20)
                return false;
            if (elem->numObjects != 1)
                return false;
        }
        else if (*elem->tagPtr == 'tstc')
        {
            if (elem->tagSize != 40)
                return false;
            if (elem->numObjects != 0)
                return false;
        }
        else
            return false;
    }
    
    if (sMemoryQueue.GetLength() != 3)
        return false;
    DebugDelete(victim);
    ValidateMemoryQueue();
    if (sMemoryQueue.GetLength() != 2)
        return false;
    DebugDelete(victim5);
    ValidateMemoryQueue();
    if (sMemoryQueue.GetLength() != 1)
        return false;
    DebugDelete(victim2);
    ValidateMemoryQueue();
    if (sMemoryQueue.GetLength() != 0)
        return false;
    DebugDelete(victim4);
    return true;
}
#endif //0

#endif // MEMORY_DEBUGGING

