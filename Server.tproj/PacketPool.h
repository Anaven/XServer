/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 */
/*
    File:       PacketPool.h

    Contains:   PacketPool class to buffer and track re-transmits of RTP packets.
    
    the ctor copies the packet data, sets a timer for the packet's age limit and
    another timer for it's possible re-transmission.
    A duration timer is started to measure the RTT based on the client's ack.
    
*/

#ifndef __PACKET_SENDER_H__
#define __PACKET_SENDER_H__

#include "Queue.h"
#include "Mutex.h"
#include "BufferPool.h"

#define PACKET_SENDER_DEBUGGING 1

class PacketEntry
{
	public:
	
		PacketEntry() : fPacketData(NULL), fPacketSize(0), fPacketLen(0), fPos(0), fAddedTime(0), 
			fExpireTime(0), fSeqNum(0), fElem(this) {}
		~PacketEntry()	{}

	    void*               fPacketData;
	    UInt32              fPacketSize;
		UInt32				fPacketLen;
		UInt32				fPos;
	    SInt64              fAddedTime;
	    SInt64              fExpireTime;
	    UInt16              fSeqNum;
		QueueElem			fElem;
};


class PacketPool
{
    public:
        
        PacketPool(UInt32 inPacketArraySize, UInt32 inPacketSize);
        ~PacketPool();
		
		int AddPacket(void* inData, const UInt32 inLength);
		PacketEntry* GetPacket();

		UInt32 IsEmpty()				{ MutexLocker theLocker(&fMutex); return fPacketsQueue.GetLength() == 0; 	};
		UInt64 GetTotalPacketsNum() 	{ return fTotalPacketsNum; 		};
		UInt64 GetDiscardPacketsNum() 	{ return fDiscardPacketsNum;	};
		UInt32 GetPacketSize()			{ return fPacketSize; 			};
		UInt32 GetPacketNum()			{ MutexLocker theLocker(&fMutex); return fPacketsQueue.GetLength();			};

		void RemovePacket(PacketEntry* theEntry);

	protected:

		PacketEntry* GetEmptyPacket();

		UInt32 	fPacketArraySize;
		UInt32	fPacketSize;

		SInt64	fLastPacketTime;
		UInt64	fTotalPacketsNum;
		UInt64	fDiscardPacketsNum;

		PacketEntry*	fPacketArray;
		Mutex			fMutex;

		Queue			fPacketsQueue;
		Queue			fEmptyPacketsQueue;
		BufferPool*		fBufferPool;
};

#endif //__PACKET_SENDER_H__

