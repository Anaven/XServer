/*
    File:       Heap.h

    Contains:   Implements a heap                   
*/

#ifndef __HEAP_H__
#define __HEAP_H__

#define _HEAP_TESTING_ 0

#include "Headers.h"

class HeapElem;

class Heap
{
    public:
    
        enum
        {
            kDefaultStartSize = 1024 //UInt32
        };
        
        Heap(UInt32 inStartSize = kDefaultStartSize);
        ~Heap();
        
        //ACCESSORS
        UInt32      	CurrentHeapSize();
        HeapElem*     	PeekMin();
        
        //MODIFIERS
        
        //These are the two primary operations supported by the heap
        //abstract data type. both run in log(n) time.
        void            Insert(HeapElem*  inElem);
        HeapElem*     	ExtractMin() { return Extract(1); }
        //removes specified element from the heap
        HeapElem*     	Remove(HeapElem* elem);
        
#if _HEAP_TESTING_
        //returns true if it passed the test, false otherwise
        static bool    	Test();
#endif
    
    private:
    
        HeapElem*     	Extract(UInt32 index);
    
#if _HEAP_TESTING_
        //verifies that the heap is in fact a heap
        void            SanityCheck(UInt32 root);
#endif
    
        HeapElem**    	fHeap;
        UInt32      	fFreeIndex;
        UInt32         	fArraySize;
};

class HeapElem
{
    public:
        HeapElem(void* enclosingObject = NULL)
            : fValue(0), fEnclosingObject(enclosingObject), fCurrentHeap(NULL) {}
        ~HeapElem() {}
        
        //This data structure emphasizes performance over extensibility
        //If it were properly object-oriented, the compare routine would
        //be virtual. However, to avoid the use of v-functions in this data
        //structure, I am assuming that the objects are compared using a 64 bit number.
        //
        void    SetValue(SInt64 newValue) { fValue = newValue; }
        SInt64  GetValue()              { return fValue; }
        void*   GetEnclosingObject()    { return fEnclosingObject; }
    	void	SetEnclosingObject(void* obj) { fEnclosingObject = obj; }
        bool  	IsMemberOfAnyHeap()     { return fCurrentHeap != NULL; }
        
    private:
    
        SInt64  fValue;
        void* fEnclosingObject;
        Heap* fCurrentHeap;
        
        friend class Heap;
};
#endif //_HEAP_H_
