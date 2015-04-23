/*
    File:       Ref.h

    Contains:   Class supports creating unique string IDs to object pointers. A grouping
                of an object and its string ID may be stored in an RefTable, and the
                associated object pointer can be looked up by string ID.
                
                Refs can only be removed from the table when no one is using the ref,
                therefore allowing clients to arbitrate access to objects in a preemptive,
                multithreaded environment. 
*/

#ifndef _REF_H_
#define _REF_H_

#include <limits.h>
#include "StrPtrLen.h"
#include "HashTable.h"
#include "Cond.h"

class RefKey;

class RefTableUtils
{
    private:

        static UInt32   HashString(StrPtrLen* inString);    

        friend class Ref;
        friend class RefKey;
};

class Ref
{
    public:

        Ref() :   fObjectP(NULL), fRefCount(0), fNextHashEntry(NULL)
        {
#if DEBUG
                fInATable = false;
                fSwapCalled = false;
#endif          
        }
        Ref(const StrPtrLen &inString, void* inObjectP)
            : fRefCount(0), fNextHashEntry(NULL)
        {   Set(inString, inObjectP); }
        ~Ref() {}
        
        void Set(const StrPtrLen& inString, void* inObjectP)
        { 
#if DEBUG
            fInATable = false;
            fSwapCalled = false;
#endif          
            fString = inString; fObjectP = inObjectP;
            fHashValue = RefTableUtils::HashString(&fString);
        }
        
#if DEBUG
        bool  IsInTable()     { return fInATable; }
#endif
        void**  GetObjectPtr()  { return &fObjectP; }
        void*   GetObject()     { return fObjectP; }
        UInt32  GetRefCount()   { return fRefCount; }
        StrPtrLen *GetString()  { return &fString; }
    private:
        
        //value
        void*   	fObjectP;
        //key
        StrPtrLen	fString;
        
        //refcounting
        UInt32  	fRefCount;
#if DEBUG
        bool  fInATable;
        bool  fSwapCalled;
#endif
        Cond  fCond;//to block threads waiting for this ref.
        
        UInt32	fHashValue;
        Ref*    fNextHashEntry;
        
        friend class RefKey;
        friend class HashTable<Ref, RefKey>;
        friend class HashTableIter<Ref, RefKey>;
        friend class RefTable;
};


class RefKey
{
public:

    //CONSTRUCTOR / DESTRUCTOR:
    RefKey(StrPtrLen* inStringP)
        :   fStringP(inStringP)
         { fHashValue = RefTableUtils::HashString(inStringP); }
            
    ~RefKey() {}
    
    
    //ACCESSORS:
    StrPtrLen*  GetString()         { return fStringP; }
    
    
private:

    //PRIVATE ACCESSORS:    
    SInt32      GetHashKey()        { return fHashValue; }

    //these functions are only used by the hash table itself. This constructor
    //will break the "Set" functions.
    RefKey(Ref *elem) : fStringP(&elem->fString),
                            fHashValue(elem->fHashValue) {}
                                    
    friend int operator ==(const RefKey &key1, const RefKey &key2)
    {
        if (key1.fStringP->Equal(*key2.fStringP))
            return true;
        return false;
    }
    
    //data:
    StrPtrLen *fStringP;
    UInt32  fHashValue;

    friend class HashTable<Ref, RefKey>;
};

typedef HashTable<Ref, RefKey> RefHashTable;
typedef HashTableIter<Ref, RefKey> RefHashTableIter;

class RefTable
{
    public:
    
        enum
        {
            kDefaultTableSize = 1193 //UInt32
        };
    
        //tableSize doesn't indicate the max number of Refs that can be added
        //(it's unlimited), but is rather just how big to make the hash table
        RefTable(UInt32 tableSize = kDefaultTableSize) : fTable(tableSize), fMutex() {}
        ~RefTable() {}
        
        //Allows access to the mutex in case you need to lock the table down
        //between operations
        Mutex*    GetMutex()      { return &fMutex; }
        RefHashTable* GetHashTable() { return &fTable; }
        
        //Registers a Ref in the table. Once the Ref is in, clients may resolve
        //the ref by using its string ID. You must setup the Ref before passing it
        //in here, ie., setup the string and object pointers
        //This function will succeed unless the string identifier is not unique,
        //in which case it will return QTSS_DupName
        //This function is atomic wrt this ref table.
        int        	Register(Ref* ref);
        
        // RegisterOrResolve
        // If the ID of the input ref is unique, this function is equivalent to
        // Register, and returns NULL.
        // If there is a duplicate ID already in the map, this funcion
        // leave it, resolves it, and returns it.
        Ref*		RegisterOrResolve(Ref* inRef);
        
        //This function may block. You can only remove a Ref from the table
        //when the refCount drops to the level specified. If several threads have
        //the ref currently, the calling thread will wait until the other threads
        //stop using the ref (by calling Release, below)
        //This function is atomic wrt this ref table.
        void        UnRegister(Ref* ref, UInt32 refCount = 0);
        
        // Same as UnRegister, but guarenteed not to block. Will return
        // true if ref was sucessfully unregistered, false otherwise
        bool      	TryUnRegister(Ref* ref, UInt32 refCount = 0);
        
        //Resolve. This function uses the provided key string to identify and grab
        //the Ref keyed by that string. Once the Ref is resolved, it is safe to use
        //(it cannot be removed from the Ref table) until you call Release. Because
        //of that, you MUST call release in a timely manner, and be aware of potential
        //deadlocks because you now own a resource being contended over.
        //This function is atomic wrt this ref table.
        Ref*      	Resolve(StrPtrLen*  inString);
        
        //Release. Release a Ref, and drops its refCount. After calling this, the
        //Ref is no longer safe to use, as it may be removed from the ref table.
        void        Release(Ref*  inRef);
        
        // Swap. This atomically removes any existing Ref in the table with the new
        // ref's ID, and replaces it with this new Ref. If there is no matching Ref
        // already in the table, this function does nothing.
        //
        // Be aware that this creates a situation where clients may have a Ref resolved
        // that is no longer in the table. The old Ref must STILL be UnRegistered normally.
        // Once Swap completes sucessfully, clients that call resolve on the ID will get
        // the new Ref object.
        void        Swap(Ref* newRef);
        
        UInt32      GetNumRefsInTable() 
        { 
        	UInt64 result =  fTable.GetNumEntries(); 
        	Assert(result < UINT_MAX); 
        	return (UInt32) result; 
        }
        
    private:
    
        
        //all this object needs to do its job is an atomic hashtable
        RefHashTable  fTable;
        Mutex         fMutex;
};


class RefReleaser
{
    public:

        RefReleaser(RefTable* inTable, Ref* inRef) : fRefTable(inTable), fRef(inRef) {}
        ~RefReleaser() { fRefTable->Release(fRef); }
        
        Ref*          GetRef() { return fRef; }
        
    private:

        RefTable*     fRefTable;
        Ref*          fRef;
};



#endif //_OSREF_H_
