/*
    File:       UDPDemuxer.cpp

    Contains:   Implements objects defined in UDPDemuxer.h

    

*/

#include "UDPDemuxer.h"

#include <errno.h>


int UDPDemuxer::RegisterTask(UInt32 inRemoteAddr, UInt16 inRemotePort,
                                        UDPDemuxerTask *inTaskP)
{
    Assert(NULL != inTaskP);
    MutexLocker locker(&fMutex);
    if (this->GetTask(inRemoteAddr, inRemotePort) != NULL)
        return EPERM;
    inTaskP->Set(inRemoteAddr, inRemotePort);
    fHashTable.Add(inTaskP);
    return 0;
}

int UDPDemuxer::UnregisterTask(UInt32 inRemoteAddr, UInt16 inRemotePort,
                                            UDPDemuxerTask *inTaskP)
{
    MutexLocker locker(&fMutex);
    //remove by executing a lookup based on key information
    UDPDemuxerTask* theTask = this->GetTask(inRemoteAddr, inRemotePort);

    if ((NULL != theTask) && (theTask == inTaskP))
    {
        fHashTable.Remove(theTask);
        return 0;
    }
    else
        return EPERM;
}

UDPDemuxerTask* UDPDemuxer::GetTask(UInt32 inRemoteAddr, UInt16 inRemotePort)
{
    UDPDemuxerKey theKey(inRemoteAddr, inRemotePort);
    return fHashTable.Map(&theKey);
}
