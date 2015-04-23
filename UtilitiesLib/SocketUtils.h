/*
    File:       SocketUtils.h

    Contains:   Some static routines for dealing with networking
                    

*/

#ifndef __SOCKETUTILS_H__
#define __SOCKETUTILS_H__

#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "Headers.h"
#include "Mutex.h"
#include "MyAssert.h"
#include "StrPtrLen.h"


class SocketUtils
{
    public:

        // Call initialize before using any socket functions.
        // (pass true for lookupDNSName if you want the hostname
        // looked up via DNS during initialization -- %%sfu)
        static void Initialize(bool lookupDNSName = true);

        //static utility routines
        static bool   	IsMulticastIPAddr(UInt32 inAddress);
    	static bool 	IsLocalIPAddr(UInt32 inAddress);

        //This function converts an integer IP address to a dotted-decimal string.
        //This function is NOT THREAD SAFE!!!
        static void ConvertAddrToString(const struct in_addr& theAddr, StrPtrLen* outAddr);
        
        // This function converts a dotted-decimal string IP address to a UInt32
        static UInt32 ConvertStringToAddr(const char* inAddr);
        
        //You can get at all the IP addrs and DNS names on this machine this way
        static UInt32       GetNumIPAddrs() { return sNumIPAddrs; }
        static inline UInt32        GetIPAddr(UInt32 inAddrIndex);
        static inline StrPtrLen*    GetIPAddrStr(UInt32 inAddrIndex);
        static inline StrPtrLen*    GetDNSNameStr(UInt32 inDNSIndex);       
        
    private:

    	struct IPAddrInfo
        {
            UInt32      	fIPAddr;
            StrPtrLen   	fIPAddrStr;
            StrPtrLen   	fDNSNameStr;
        };

    	static IPAddrInfo*	sIPAddrInfoArray;
        static UInt32    	sNumIPAddrs;
        static Mutex      	sMutex;
};

inline UInt32 SocketUtils::GetIPAddr(UInt32 inAddrIndex)
{
    Assert(sIPAddrInfoArray != NULL);
    Assert(inAddrIndex < sNumIPAddrs);
    return sIPAddrInfoArray[inAddrIndex].fIPAddr;
}

inline StrPtrLen* SocketUtils::GetIPAddrStr(UInt32 inAddrIndex)
{
    Assert(sIPAddrInfoArray != NULL);
    Assert(inAddrIndex < sNumIPAddrs);
    return &sIPAddrInfoArray[inAddrIndex].fIPAddrStr;
}

inline StrPtrLen* SocketUtils::GetDNSNameStr(UInt32 inDNSIndex)
{
    Assert(sIPAddrInfoArray != NULL);
    Assert(inDNSIndex < sNumIPAddrs);
    return &sIPAddrInfoArray[inDNSIndex].fDNSNameStr;
}

#endif // __SOCKETUTILS_H__

