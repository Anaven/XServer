/*
    File:       Heap.cpp

    Contains:   Implements a heap
*/

#include <string.h>

#include "Heap.h"
#include "Memory.h"

Heap::Heap(UInt32 inStartSize)
: fFreeIndex(1)
{
    if (inStartSize < 2)
        fArraySize = 2;
    else
        fArraySize = inStartSize;
   
    fHeap = NEW HeapElem*[fArraySize];  
}


Heap::~Heap() 
{ 
    if (fHeap != NULL) 
        delete fHeap; 
}


UInt32 Heap::CurrentHeapSize() 
{ 
	return fFreeIndex - 1; 
}


HeapElem* Heap::PeekMin() 
{ 
	if (CurrentHeapSize() > 0) 
    	return fHeap[1]; 

	return NULL; 
}


void Heap::Insert(HeapElem* inElem)
{
    //Assert(inElem != NULL);
    
    if ((fHeap == NULL) || (fFreeIndex == fArraySize))
    {
        fArraySize *= 2;
        HeapElem** tempArray = NEW HeapElem*[fArraySize];
        if ((fHeap != NULL) && (fFreeIndex > 1))
            ::memcpy(tempArray, fHeap, sizeof(HeapElem*) * fFreeIndex);
            
        delete [] fHeap;
        fHeap = tempArray;
   } 
    
    //Assert(fHeap != NULL);
    //Assert(inElem->fCurrentHeap == NULL);
    //Assert(fArraySize > fFreeIndex);
    
#if _Heap_TESTING_
    SanityCheck(1);
#endif

    //insert the element into the last leaf of the tree
    fHeap[fFreeIndex] = inElem;
    
    //bubble the new element up to its proper place in the heap
    
    //start at the last leaf of the tree
    UInt32 swapPos = fFreeIndex;
    while (swapPos > 1)
    {
        //move up the chain until we get to the root, bubbling this new element
        //to its proper place in the tree
        UInt32 nextSwapPos = swapPos >> 1;
        
        //if this child is greater than it's parent, we need to do the old
        //switcheroo
        if (fHeap[swapPos]->fValue < fHeap[nextSwapPos]->fValue)
        {
            HeapElem* temp = fHeap[swapPos];
            fHeap[swapPos] = fHeap[nextSwapPos];
            fHeap[nextSwapPos] = temp;
            swapPos = nextSwapPos;
        }
        else
            //if not, we are done!
            break;
    }
    inElem->fCurrentHeap = this;
    fFreeIndex++;
}


HeapElem* Heap::Extract(UInt32 inIndex)
{
    if ((fHeap == NULL) || (fFreeIndex <= inIndex))
        return NULL;
        
#if _Heap_TESTING_
    SanityCheck(1);
#endif
    
    //store a reference to the element we want to extract
    HeapElem* victim = fHeap[inIndex];
    //Assert(victim->fCurrentHeap == this);
    victim->fCurrentHeap = NULL;
    
    //but now we need to preserve this heuristic. We do this by taking
    //the last leaf, putting it at the empty position, then heapifying that chain
    fHeap[inIndex] = fHeap[fFreeIndex - 1];
    fFreeIndex--;
    
    //The following is an implementation of the Heapify algorithm (CLR 7.1 pp 143)
    //The gist is that this new item at the top of the heap needs to be bubbled down
    //until it is bigger than its two children, therefore maintaining the heap property.
    
    UInt32 parent = inIndex;
    while (parent < fFreeIndex)
    {
        //which is bigger? parent or left child?
        UInt32 greatest = parent;
        UInt32 leftChild = parent * 2;
        if ((leftChild < fFreeIndex) && (fHeap[leftChild]->fValue < fHeap[parent]->fValue))
            greatest = leftChild;

        //which is bigger? the biggest so far or the right child?
        UInt32 rightChild = (parent * 2) + 1;
        if ((rightChild < fFreeIndex) && (fHeap[rightChild]->fValue < fHeap[greatest]->fValue))
            greatest = rightChild;
         
        //if the parent is in fact bigger than its two children, we have bubbled
        //this element down far enough
        if (greatest == parent)
            break;
            
        //parent is not bigger than at least one of its two children, so swap the parent
        //with the largest item.
        HeapElem* temp = fHeap[parent];
        fHeap[parent] = fHeap[greatest];
        fHeap[greatest] = temp;
        
        //now heapify the remaining chain
        parent = greatest;
    }
    
    return victim;
}


HeapElem* Heap::Remove(HeapElem* elem)
{
    if ((fHeap == NULL) || (fFreeIndex == 1))
        return NULL;
        
#if _Heap_TESTING_
    SanityCheck(1);
#endif

    //first attempt to locate this element in the heap
    UInt32 theIndex = 1;
    for ( ; theIndex < fFreeIndex; theIndex++)
        if (elem == fHeap[theIndex])
            break;
            
    //either we've found it, or this is a bogus element
    if (theIndex == fFreeIndex)
        return NULL;
        
    return Extract(theIndex);
}


#if _Heap_TESTING_

void Heap::SanityCheck(UInt32 root)
{
    //make sure root is greater than both its children. Do so recursively
    if (root < fFreeIndex)
    {
        if ((root * 2) < fFreeIndex)
        {
            Assert(fHeap[root]->fValue <= fHeap[root * 2]->fValue);
            SanityCheck(root * 2);
        }
        if (((root * 2) + 1) < fFreeIndex)
        {
            Assert(fHeap[root]->fValue <= fHeap[(root * 2) + 1]->fValue);
            SanityCheck((root * 2) + 1);
        }
    }
}


bool Heap::Test()
{
    Heap victim(2);
    HeapElem elem1;
    HeapElem elem2;
    HeapElem elem3;
    HeapElem elem4;
    HeapElem elem5;
    HeapElem elem6;
    HeapElem elem7;
    HeapElem elem8;
    HeapElem elem9;

    HeapElem* max = victim.ExtractMin();
    if (max != NULL)
        return false;
        
    elem1.SetValue(100);
    victim.Insert(&elem1);
    
    max = victim.ExtractMin();
    if (max != &elem1)
        return false;
    max = victim.ExtractMin();
    if (max != NULL)
        return false;
    
    elem1.SetValue(100);
    elem2.SetValue(80);
    
    victim.Insert(&elem1);
    victim.Insert(&elem2);
    
    max = victim.ExtractMin();
    if (max != &elem2)
        return false;
    max = victim.ExtractMin();
    if (max != &elem1)
        return false;
    max = victim.ExtractMin();
    if (max != NULL)
        return false;
    
    victim.Insert(&elem2);
    victim.Insert(&elem1);

    max = victim.ExtractMin();
    if (max != &elem2)
        return false;
    max = victim.ExtractMin();
    if (max != &elem1)
        return false;
        
    elem3.SetValue(70);
    elem4.SetValue(60);

    victim.Insert(&elem3);
    victim.Insert(&elem1);
    victim.Insert(&elem2);
    victim.Insert(&elem4);
    
    max = victim.ExtractMin();
    if (max != &elem4)
        return false;
    max = victim.ExtractMin();
    if (max != &elem3)
        return false;
    max = victim.ExtractMin();
    if (max != &elem2)
        return false;
    max = victim.ExtractMin();
    if (max != &elem1)
        return false;

    elem5.SetValue(50);
    elem6.SetValue(40);
    elem7.SetValue(30);
    elem8.SetValue(20);
    elem9.SetValue(10);

    victim.Insert(&elem5);
    victim.Insert(&elem3);
    victim.Insert(&elem1);
    
    max = victim.ExtractMin();
    if (max != &elem5)
        return false;
    
    victim.Insert(&elem4);
    victim.Insert(&elem2);

    max = victim.ExtractMin();
    if (max != &elem4)
        return false;
    max = victim.ExtractMin();
    if (max != &elem3)
        return false;
    
    victim.Insert(&elem2);

    max = victim.ExtractMin();
    if (max != &elem2)
        return false;

    victim.Insert(&elem2);
    victim.Insert(&elem6);

    max = victim.ExtractMin();
    if (max != &elem6)
        return false;

    victim.Insert(&elem6);
    victim.Insert(&elem3);
    victim.Insert(&elem4);
    victim.Insert(&elem5);

    max = victim.ExtractMin();
    if (max != &elem6)
        return false;
    max = victim.ExtractMin();
    if (max != &elem5)
        return false;

    victim.Insert(&elem8);
    max = victim.ExtractMin();
    if (max != &elem8)
        return false;
    max = victim.ExtractMin();
    if (max != &elem4)
        return false;
        
    victim.Insert(&elem5);
    victim.Insert(&elem4);
    victim.Insert(&elem9);
    victim.Insert(&elem7);
    victim.Insert(&elem8);
    victim.Insert(&elem6);

    max = victim.ExtractMin();
    if (max != &elem9)
        return false;
    max = victim.ExtractMin();
    if (max != &elem8)
        return false;
    max = victim.ExtractMin();
    if (max != &elem7)
        return false;
    max = victim.ExtractMin();
    if (max != &elem6)
        return false;
    max = victim.ExtractMin();
    if (max != &elem5)
        return false;
    max = victim.ExtractMin();
    if (max != &elem4)
        return false;
    max = victim.ExtractMin();
    if (max != &elem3)
        return false;
    max = victim.ExtractMin();
    if (max != &elem2)
        return false;
    max = victim.ExtractMin();
    if (max != &elem2)
        return false;
    max = victim.ExtractMin();
    if (max != &elem1)
        return false;
    max = victim.ExtractMin();
    if (max != NULL)
        return false;
        
    victim.Insert(&elem1);
    victim.Insert(&elem2);
    victim.Insert(&elem3);
    victim.Insert(&elem4);
    victim.Insert(&elem5);
    victim.Insert(&elem6);
    victim.Insert(&elem7);
    victim.Insert(&elem8);
    victim.Insert(&elem9);
    
    max = victim.Remove(&elem7);
    if (max != &elem7)
        return false;
    max = victim.Remove(&elem9);
    if (max != &elem9)
        return false;
    max = victim.ExtractMin();
    if (max != &elem8)
        return false;
    max = victim.Remove(&elem2);
    if (max != &elem2)
        return false;
    max = victim.Remove(&elem2);
    if (max != NULL)
        return false;
    max = victim.Remove(&elem8);
    if (max != NULL)
        return false;
    max = victim.Remove(&elem5);
    if (max != &elem5)
        return false;
    max = victim.Remove(&elem6);
    if (max != &elem6)
        return false;
    max = victim.Remove(&elem1);
    if (max != &elem1)
        return false;
    max = victim.ExtractMin();
    if (max != &elem4)
        return false;
    max = victim.Remove(&elem1);
    if (max != NULL)
        return false;
    
    return true;
}
#endif
