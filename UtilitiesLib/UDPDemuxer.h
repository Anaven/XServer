/*
    File:       UDPDemuxer.h

    Contains:   Provides a "Listener" socket for UDP. Blocks on a local IP & port,
                waiting for data. When it gets data, it passes it off to a UDPDemuxerTask
                object depending on where it came from.

    
*/

#ifndef __UDPDEMUXER_H__
#define __UDPDEMUXER_H__

#include "Headers.h"
#include "Mutex.h"
#include "HashTable.h"
#include "StrPtrLen.h"

class Task;
class UDPDemuxerKey;

//IMPLEMENTATION ONLY:
//HASH TABLE CLASSES USED ONLY IN IMPLEMENTATION


class UDPDemuxerUtils
{
    private:
    
        static UInt32 ComputeHashValue(UInt32 inRemoteAddr, UInt16 inRemotePort)
        { return ((inRemoteAddr << 16) + inRemotePort); }
            
    	friend class UDPDemuxerTask;
    	friend class UDPDemuxerKey;
};

class UDPDemuxerTask
{
    public:
    
        UDPDemuxerTask()
            :   fRemoteAddr(0), fRemotePort(0),
                fHashValue(0), fNextHashEntry(NULL) {}
        virtual ~UDPDemuxerTask() {}
        
        UInt32  GetRemoteAddr() { return fRemoteAddr; }

		//key values
        UInt32 fRemoteAddr;
        UInt16 fRemotePort;
        
    private:

        void Set(UInt32 inRemoteAddr, UInt16 inRemotePort)
        {   
        	fRemoteAddr = inRemoteAddr; fRemotePort = inRemotePort;
            fHashValue = UDPDemuxerUtils::ComputeHashValue(fRemoteAddr, fRemotePort);
        }
        
        //precomputed for performance
        UInt32 fHashValue;
        
        UDPDemuxerTask  *fNextHashEntry;

        friend class UDPDemuxerKey;
        friend class UDPDemuxer;
        friend class HashTable<UDPDemuxerTask,UDPDemuxerKey>;
};



class UDPDemuxerKey
{
    private:

        //CONSTRUCTOR / DESTRUCTOR:
        UDPDemuxerKey(UInt32 inRemoteAddr, UInt16 inRemotePort)
            :   fRemoteAddr(inRemoteAddr), fRemotePort(inRemotePort)
        { fHashValue = UDPDemuxerUtils::ComputeHashValue(inRemoteAddr, inRemotePort); }
                
        ~UDPDemuxerKey() {}
        
        
    private:

        //PRIVATE ACCESSORS:    
        UInt32      GetHashKey()        { return fHashValue; }

        //these functions are only used by the hash table itself. This constructor
        //will break the "Set" functions.
        UDPDemuxerKey(UDPDemuxerTask *elem) :   fRemoteAddr(elem->fRemoteAddr),
                                                fRemotePort(elem->fRemotePort), 
                                                fHashValue(elem->fHashValue) {}
                                            
        friend int operator ==(const UDPDemuxerKey &key1, const UDPDemuxerKey &key2) {
            if ((key1.fRemoteAddr == key2.fRemoteAddr) &&
                (key1.fRemotePort == key2.fRemotePort))
                return true;
            return false;
        }
        
        //data:
        UInt32 fRemoteAddr;
        UInt16 fRemotePort;
        UInt32 fHashValue;

        friend class HashTable<UDPDemuxerTask,UDPDemuxerKey>;
        friend class UDPDemuxer;
};

//CLASSES USED ONLY IN IMPLEMENTATION
typedef HashTable<UDPDemuxerTask, UDPDemuxerKey> UDPDemuxerHashTable;

class UDPDemuxer
{
    public:

        UDPDemuxer() : fHashTable(kMaxHashTableSize), fMutex() {}
        ~UDPDemuxer() {}

        //These functions grab the mutex and are therefore premptive safe
        
        // Return values: 0, or EPERM if there is already a task registered
        // with this address combination
        int RegisterTask(UInt32 inRemoteAddr, UInt16 inRemotePort,
                                        UDPDemuxerTask *inTaskP);

        // Return values: 0, or EPERM if this task / address combination
        // is not registered
        int UnregisterTask(UInt32 inRemoteAddr, UInt16 inRemotePort,
                                        UDPDemuxerTask *inTaskP);
        
        //Assumes that parent has grabbed the mutex!
        UDPDemuxerTask* GetTask(UInt32 inRemoteAddr, UInt16 inRemotePort);

        bool  AddrInMap(UInt32 inRemoteAddr, UInt16 inRemotePort)
        { return (this->GetTask(inRemoteAddr, inRemotePort) != NULL); }
                    
        Mutex*                	GetMutex()      { return &fMutex; }
        UDPDemuxerHashTable*	GetHashTable()  { return &fHashTable; }
        
    private:
    
        enum
        {
            kMaxHashTableSize = 2747//is this prime? it should be... //UInt32
        };
        UDPDemuxerHashTable fHashTable;
        Mutex             	fMutex;//this data structure is shared!
};

#endif // __UDPDEMUXER_H__


