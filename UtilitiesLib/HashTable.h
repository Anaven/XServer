/*
    File:       HashTable.h

    Contains:  Defines a template class for hash tables.
*/

#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include "Headers.h"
#include "Memory.h"
#include "MyAssert.h"

/*
T must have a fNextHashEntry field, and key(T) must returns the key of type K.
K must have a method GetHashKey() that returns an UInt32 bit hash value.
Will the hash table can contain duplicate keys, the Map function will return only the first one.
*/

template<class T, class K>
class HashTable {
public:
    HashTable( UInt32 size )
    {
        fHashTable = new ( T*[size] );
        Assert( fHashTable );
        memset( fHashTable, 0, sizeof(T*) * size );
        fSize = size;
        // Determine whether the hash size is a power of 2
        // if not set the mask to zero, otherwise we can
        // use the mask which is faster for ComputeIndex
        fMask = fSize - 1;
        if ((fMask & fSize) != 0)
            fMask = 0;
        fNumEntries = 0;
    }
    ~HashTable()
    {
        delete [] fHashTable;
    }
    void Add( T* entry ) {
        Assert( entry->fNextHashEntry == NULL );
        K key( entry );
        UInt32 theIndex = ComputeIndex( key.GetHashKey() );
        entry->fNextHashEntry = fHashTable[ theIndex ];
        fHashTable[ theIndex ] = entry;
        fNumEntries++;
    }
    void Remove( T* entry )
    {
        K key( entry );
        UInt32 theIndex = ComputeIndex( key.GetHashKey() );
        T* elem = fHashTable[ theIndex ];
        T* last = NULL;
        while (elem && elem != entry) {
            last = elem;
            elem = elem->fNextHashEntry;
        }
    
        if ( elem ) // sometimes remove is called 2x ( swap, then un register )
        {
            Assert(elem);
            if (last)
                last->fNextHashEntry = elem->fNextHashEntry;
            else
                fHashTable[ theIndex ] = elem->fNextHashEntry;
            elem->fNextHashEntry = NULL;
            fNumEntries--;
        }
    }
    T* Map( K* key )
    {
        UInt32 theIndex = ComputeIndex( key->GetHashKey() );
        T* elem = fHashTable[ theIndex ];
        while (elem) {
            K elemKey( elem );
            if (elemKey == *key)
                break;
            elem = elem->fNextHashEntry;
        }
        return elem;
    }
    UInt64 GetNumEntries() { return fNumEntries; }
    
    UInt32 GetTableSize() { return fSize; }
    T* GetTableEntry( int i ) { return fHashTable[i]; }

private:
    T** fHashTable;
    UInt32 fSize;
    UInt32 fMask;
    UInt64 fNumEntries;
    
    UInt32 ComputeIndex( UInt32 hashKey )
    {
        if (fMask)
            return( hashKey & fMask );
        else
            return( hashKey % fSize );
    }
};

template<class T, class K>
class HashTableIter {
public:
    HashTableIter( HashTable<T,K>* table )
    {
        fHashTable = table;
        First();
    }
    void First()
    {
        for (fIndex = 0; fIndex < fHashTable->GetTableSize(); fIndex++) {
            fCurrent = fHashTable->GetTableEntry( fIndex );
            if (fCurrent)
                break;
        }
    }
    void Next()
    {
        fCurrent = fCurrent->fNextHashEntry;
        if (!fCurrent) {
            for (fIndex = fIndex + 1; fIndex < fHashTable->GetTableSize(); fIndex++) {
                fCurrent = fHashTable->GetTableEntry( fIndex );
                if (fCurrent)
                    break;
            }
        }
    }
    bool IsDone()
    {
        return( fCurrent == NULL );
    }
    T* GetCurrent() { return fCurrent; }
    
private:
    HashTable<T,K>* fHashTable;
    T* fCurrent;
    UInt32 fIndex;
};
#endif //_OSHASHTABLE_H_
