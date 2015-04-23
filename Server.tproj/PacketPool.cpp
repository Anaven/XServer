/*
    File:       PacketPool.cpp

    Contains:   PacketPool class to buffer and track re-transmits of RTP packets.
    
    
*/

#include "OS.h"
#include "PacketPool.h"
#include "Mutex.h"
#include "Memory.h"

PacketPool::PacketPool(UInt32 inPacketArraySize, UInt32 inPacketSize)
:   fPacketArraySize(inPacketArraySize),
    fPacketSize(inPacketSize),
    fLastPacketTime(0),
    fTotalPacketsNum(0),
    fDiscardPacketsNum(0)
{
    MutexLocker theLocker(&fMutex);

    fPacketArray = NEW PacketEntry [sizeof(PacketEntry) * fPacketArraySize];
	for (UInt32 i = 0; i < fPacketArraySize; i++)
	{
		fPacketArray[i].fPacketLen	= 0;
		fPacketArray[i].fPacketSize	= 0;
		fPacketArray[i].fPos		= 0;
		fPacketArray[i].fAddedTime	= 0;
		fPacketArray[i].fPacketData = NEW char[fPacketSize];
		
		fEmptyPacketsQueue.EnQueue(&fPacketArray[i].fElem);
	}
}

PacketPool::~PacketPool()
{
    MutexLocker theLocker(&fMutex);
	QueueElem *theElem = NULL;


	while ((theElem = fEmptyPacketsQueue.DeQueue()) != NULL) {}
	
	while ((theElem = fPacketsQueue.DeQueue()) != NULL) {}

	for (UInt32 i = 0; i < fPacketArraySize; i++)
	{
		Assert(fPacketArray[i].fElem.InQueue() == NULL);
		delete [] (char*)fPacketArray[i].fPacketData;
	}

    delete [] fPacketArray;
}

int PacketPool::AddPacket(void* inData, UInt32 inLength)
{
    MutexLocker theLocker(&fMutex);

    if (inLength > fPacketSize)
    {
        fDiscardPacketsNum++;
        return inLength - fPacketSize;
    }

    PacketEntry* theEntry = GetEmptyPacket();
    if (theEntry == NULL)
    {
    	fDiscardPacketsNum++;
        theEntry = this->GetPacket();
        if (theEntry == NULL) 
            return -1;
    }

    memcpy(theEntry->fPacketData, inData, inLength);
    theEntry->fPacketLen    = inLength;
    theEntry->fPacketSize   = fPacketSize;
    theEntry->fPos          = 0;
    theEntry->fAddedTime    = 0; //OS::Milliseconds();

    fPacketsQueue.EnQueue(&theEntry->fElem);

    return 0;
}

PacketEntry* PacketPool::GetPacket()
{
    MutexLocker theLocker(&fMutex);

    if (fPacketsQueue.GetLength() <= 0)
        return NULL;

    return (PacketEntry*)fPacketsQueue.DeQueue()->GetEnclosingObject();
}

PacketEntry* PacketPool::GetEmptyPacket()
{
    MutexLocker theLocker(&fMutex);

	QueueElem *theElem = fEmptyPacketsQueue.DeQueue();
	if (theElem == NULL)
		return NULL;
	
    return (PacketEntry*)theElem->GetEnclosingObject();
}

void PacketPool::RemovePacket(PacketEntry* theEntry)
{
    MutexLocker theLocker(&fMutex);

    Assert(theEntry != NULL);
	theEntry->fPacketLen    = 0;
    theEntry->fPacketSize   = 0;
    theEntry->fPos          = 0;
    theEntry->fAddedTime    = 0;
	
    fEmptyPacketsQueue.EnQueue(&theEntry->fElem);
}

