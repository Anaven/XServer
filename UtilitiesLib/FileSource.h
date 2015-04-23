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
    File:       FileSource.h

    Contains:   simple file abstraction. This file abstraction is ONLY to be
                used for files intended for serving 
                    
    
*/

#ifndef __FILE_SOURCE_H_
#define __FILE_SOURCE_H_

#include <stdio.h>
#include <time.h>

#include "Headers.h"
#include "StrPtrLen.h"
#include "Queue.h"

#define READ_LOG 0

class FileBlockBuffer 
{

 public:
 	
	FileBlockBuffer(): fArrayIndex(-1),fBufferSize(0),fBufferFillSize(0),fDataBuffer(NULL),fDummy(0){}
	~FileBlockBuffer(void);
	void AllocateBuffer(UInt32 buffSize);
	void TestBuffer(void);
	void CleanBuffer() { ::memset(fDataBuffer,0, fBufferSize); }
	void SetFillSize(UInt32 fillSize) {fBufferFillSize = fillSize;}
	UInt32 GetFillSize(void) { return fBufferFillSize;}
	QueueElem *GetQElem() { return &fQElem; }
	SInt64              fArrayIndex;
	UInt32              fBufferSize;
	UInt32              fBufferFillSize;
	char                *fDataBuffer;
	QueueElem         	fQElem;
	UInt32              fDummy;
};



class FileBlockPool
{
	    enum 
		{
            kDataBufferUnitSizeExp      = 15,// base 2 exponent
            kBufferUnitSize             = (1 << kDataBufferUnitSizeExp ) // 32Kbytes
	    };

    public:
        FileBlockPool(void) :  fMaxBuffers(1),  fNumCurrentBuffers(0), fBufferUnitSizeBytes(kBufferUnitSize){}
        ~FileBlockPool(void);
        
        void SetMaxBuffers(UInt32 maxBuffers) { if (maxBuffers > 0) fMaxBuffers = maxBuffers; }

        void SetBuffIncValue(UInt32 bufferInc) { if (bufferInc > 0) fBufferInc = bufferInc;}
        void IncMaxBuffers(void) { fMaxBuffers += fBufferInc; }
        void DecMaxBuffers(void) { if (fMaxBuffers > fBufferInc) fMaxBuffers-= fBufferInc; }
        void DecCurBuffers(void) { if (fNumCurrentBuffers > 0) fNumCurrentBuffers--; }
        
        void SetBufferUnitSize  (UInt32 inUnitSizeInK)      { fBufferUnitSizeBytes = inUnitSizeInK * 1024; }
        UInt32 GetBufferUnitSizeBytes()     { return fBufferUnitSizeBytes; }
        UInt32 GetMaxBuffers(void)  { return fMaxBuffers; }
        UInt32 GetIncBuffers()      { return fBufferInc; }
        UInt32 GetNumCurrentBuffers(void)   { return fNumCurrentBuffers; }
        void DeleteBlockPool();
        FileBlockBuffer* GetBufferElement(UInt32 bufferSizeBytes);
        void MarkUsed(FileBlockBuffer* inBuffPtr);

    private:
        Queue 	fQueue;
        UInt32  fMaxBuffers;
        UInt32  fNumCurrentBuffers; 
        UInt32  fBufferInc; 
        UInt32  fBufferUnitSizeBytes;
        UInt32  fBufferDataSizeBytes;

};

class FileMap 
{

    public:
        FileMap(void):fFileMapArray(NULL),fDataBufferSize(0),fMapArraySize(0),fNumBuffSizeUnits(0) {}
        ~FileMap(void) {fFileMapArray = NULL;}
        void    AllocateBufferMap(UInt32 inUnitSizeInK, UInt32 inNumBuffSizeUnits, UInt32 inBufferIncCount, UInt32 inMaxBitRateBuffSizeInBlocks, UInt64 fileLen, UInt32 inBitRate);
        char*   GetBuffer(SInt64 bufIndex, bool *outIsEmptyBuff);
        void    TestBuffer(SInt32 bufIndex) {Assert (bufIndex >= 0); fFileMapArray[bufIndex]->TestBuffer();};
        void    SetIndexBuffFillSize(SInt32 bufIndex, UInt32 fillSize) { Assert (bufIndex >= 0); fFileMapArray[bufIndex]->SetFillSize(fillSize);}
        UInt32  GetMaxBufSize(void) {return fDataBufferSize;}
        UInt32  GetBuffSize(SInt64 bufIndex)    { Assert (bufIndex >= 0); return fFileMapArray[bufIndex]->GetFillSize(); }
        UInt32  GetIncBuffers(void) { return fBlockPool.GetIncBuffers(); }
        void    IncMaxBuffers()     {fBlockPool.IncMaxBuffers(); }
        void    DecMaxBuffers()     {fBlockPool.DecMaxBuffers(); }
        bool  Initialized()       { return fFileMapArray == NULL ? false : true; }
        void    Clean(void);
        void    DeleteMap(void);
        void    DeleteOldBuffs(void);
        SInt64  GetBuffIndex(UInt64 inPosition) {   return inPosition / this->GetMaxBufSize();  }
        SInt64  GetMaxBuffIndex() { Assert(fMapArraySize > 0); return fMapArraySize -1; }
        UInt64  GetBuffOffset(SInt64 bufIndex) { return (UInt64) (bufIndex * this->GetMaxBufSize() ); }
        FileBlockPool fBlockPool;

        FileBlockBuffer**   fFileMapArray;
    
    private:

        UInt32              fDataBufferSize;
        SInt64              fMapArraySize;
        UInt32              fNumBuffSizeUnits;
    
};

class FileSource
{
    public:
    
        FileSource() :    fFile(-1), fLength(0), fPosition(0), fReadPos(0), fWritePos(0), fShouldClose(true), fIsDir(false), fCacheEnabled(false)                       
        {
        #if READ_LOG 
            fFileLog = NULL;
            fTrackID = 0;
            fFilePath[0]=0;
        #endif
        }
                
        FileSource(const char *inPath) :  fFile(-1), fLength(0), fPosition(0), fReadPos(0), fWritePos(0), fShouldClose(true), fIsDir(false),fCacheEnabled(false)
        {
         	Set(inPath); 
         
        #if READ_LOG 
            fFileLog = NULL;
            fTrackID = 0;
            fFilePath[0]=0;
		#endif      
		}
        
        ~FileSource() { Close();  fFileMap.DeleteMap();}
        
        //Sets this object to reference this file
        void	Set(const char *inPath);
        
        // Call this if you don't want Close or the destructor to close the fd
        void	DontCloseFD() { fShouldClose = false; }
        
        //Advise: this advises the OS that we are going to be reading soon from the
        //following position in the file
        void	Advise(UInt64 advisePos, UInt32 adviseAmt);
        
        int    	Read(void* inBuffer, UInt32 inLength, UInt32* outRcvLen = NULL)
	            {   
	            	return ReadFromDisk(inBuffer, inLength, outRcvLen);
	            }
                    
        int    	Read(UInt64 inPosition, void* inBuffer, UInt32 inLength, UInt32* outRcvLen = NULL);
        int    	ReadFromDisk(void* inBuffer, UInt32 inLength, UInt32* outRcvLen = NULL);
        int    	ReadFromCache(UInt64 inPosition, void* inBuffer, UInt32 inLength, UInt32* outRcvLen = NULL);
        int    	ReadFromPos(UInt64 inPosition, void* inBuffer, UInt32 inLength, UInt32* outRcvLen = NULL);

        int    	Write(void* inBuffer, UInt32 inLength, UInt32* outSndLen = NULL)
				{	
					return WriteToDisk(inBuffer, inLength, outSndLen);
				}

		int 	WriteToDisk(void* inBuffer, UInt32 inLength, UInt32* outSndLen = NULL);


        void 	EnableFileCache(bool enabled) {MutexLocker locker(&fMutex); fCacheEnabled = enabled; }
        bool	GetCacheEnabled() { return fCacheEnabled; }
        void	AllocateFileCache(UInt32 inUnitSizeInK = 32, UInt32 bufferSizeUnits = 0, UInt32 incBuffers = 1, UInt32 inMaxBitRateBuffSizeInBlocks = 8, UInt32 inBitRate = 32768) 
                {   
                	fFileMap.AllocateBufferMap(inUnitSizeInK, bufferSizeUnits,incBuffers, inMaxBitRateBuffSizeInBlocks, fLength, inBitRate);
                } 
        void	IncMaxBuffers()     {MutexLocker locker(&fMutex); fFileMap.IncMaxBuffers(); }
        void	DecMaxBuffers()     {MutexLocker locker(&fMutex); fFileMap.DecMaxBuffers(); }

        int    	FillBuffer(char* ioBuffer, char *buffStart, SInt32 bufIndex);
                
        void 	Close();
        time_t  GetModDate()                { return fModDate; }
        UInt64	GetLength()                 { return fLength; }
        UInt64	GetCurOffset()              { return fPosition; }
        void 	Seek(SInt64 newPosition)    { fPosition = newPosition;  }
        bool 	IsValid()                        	{ return fFile != -1;       }
        bool 	IsDir()        						{ return fIsDir; }
        
        // For async I/O purposes
        int		GetFD()                     		{ return fFile; }
        void	SetTrackID(UInt32 trackID);
        // So that close won't do anything
        void 	ResetFD()  							{ fFile=-1; }

        void 	SetLog(const char *inPath);
    
    private:

        int     fFile;
        UInt64  fLength;
        UInt64  fPosition;
        UInt64  fReadPos;
		UInt64	fWritePos;
        bool  	fShouldClose;
        bool  	fIsDir;
        time_t  fModDate;
        
        
        Mutex 	fMutex;
        FileMap fFileMap;
        bool  	fCacheEnabled;
#if READ_LOG
        FILE*               fFileLog;
        char                fFilePath[1024];
        UInt32              fTrackID;
#endif

};

#endif //__FILE_SOURCE_H_
