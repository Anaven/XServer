/*
    File:       SocketUtils.cpp

    Contains:   Implements utility functions defined in SocketUtils.h
                    

    
    
*/

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>

#include "SocketUtils.h"

UInt32 SocketUtils::sNumIPAddrs = 0;
SocketUtils::IPAddrInfo* SocketUtils::sIPAddrInfoArray = NULL;
Mutex SocketUtils::sMutex;

void SocketUtils::Initialize(bool lookupDNSName)
{
    static const UInt32 kMaxAddrBufferSize = 2048;
    
    struct ifconf ifc;
    ::memset(&ifc,0,sizeof(ifc));
    struct ifreq* ifr;
    char buffer[kMaxAddrBufferSize];
    
    int tempSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (tempSocket == -1)
        return;
        
    ifc.ifc_len = kMaxAddrBufferSize;
    ifc.ifc_buf = buffer;

    int err = ::ioctl(tempSocket, SIOCGIFCONF, (char*)&ifc);
    if (err == -1)
        return;

    ::close(tempSocket);
    tempSocket = -1;

    //walk through the list of IP addrs twice. Once to find out how many,
    //the second time to actually grab their information
    char* ifReqIter = NULL;
    sNumIPAddrs = 0;

    for (ifReqIter = buffer; ifReqIter < (buffer + ifc.ifc_len);)
    {
        ifr = (struct ifreq*)ifReqIter;
        ifReqIter += sizeof(struct ifreq);
        
        // Some platforms have lo as the first interface, so we have code to
        // work around this problem below
        //if (::strncmp(ifr->ifr_name, "lo", 2) == 0)
        //  Assert(sNumIPAddrs > 0); // If the loopback interface is interface 0, we've got problems
    
        //Only count interfaces in the AF_INET family.
        if (ifr->ifr_addr.sa_family == AF_INET)
            sNumIPAddrs++;
    }

    sIPAddrInfoArray = new IPAddrInfo[sNumIPAddrs];
    ::memset(sIPAddrInfoArray, 0, sizeof(struct IPAddrInfo) * sNumIPAddrs);
    UInt32 currentIndex = 0;

    for (ifReqIter = buffer; ifReqIter < (buffer + ifc.ifc_len);)
    {
        ifr = (struct ifreq*)ifReqIter;
        ifReqIter += sizeof(struct ifreq);

        //Only count interfaces in the AF_INET family.
        if (ifr->ifr_addr.sa_family == AF_INET)
        {
            struct sockaddr_in* addrPtr = (struct sockaddr_in*)&ifr->ifr_addr;  
            char* theAddrStr = ::inet_ntoa(addrPtr->sin_addr);

            //store the IP addr
            sIPAddrInfoArray[currentIndex].fIPAddr = ntohl(addrPtr->sin_addr.s_addr);

            //store the IP addr as a string
            sIPAddrInfoArray[currentIndex].fIPAddrStr.Len = ::strlen(theAddrStr);
            sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fIPAddrStr.Len + 2];
            ::strcpy(sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr, theAddrStr);

            struct hostent* theDNSName = NULL;
            if (lookupDNSName) //convert this addr to a dns name, and store it
                theDNSName = ::gethostbyaddr((char *)&addrPtr->sin_addr, sizeof(addrPtr->sin_addr), AF_INET);

            if (theDNSName != NULL)
            {
                sIPAddrInfoArray[currentIndex].fDNSNameStr.Len = ::strlen(theDNSName->h_name);
                sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fDNSNameStr.Len + 2];
                ::strcpy(sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr, theDNSName->h_name);
            }
            else
            {
                //if we failed to look up the DNS name, just store the IP addr as a string
                sIPAddrInfoArray[currentIndex].fDNSNameStr.Len = sIPAddrInfoArray[currentIndex].fIPAddrStr.Len;
                sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr = new char[sIPAddrInfoArray[currentIndex].fDNSNameStr.Len + 2];
                ::strcpy(sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr, sIPAddrInfoArray[currentIndex].fIPAddrStr.Ptr);
            }

            printf("SockUtil: %s %s %s\n", ifr->ifr_name, theAddrStr, sIPAddrInfoArray[currentIndex].fDNSNameStr.Ptr);
        }

         currentIndex++;
    }

    Assert(currentIndex == sNumIPAddrs);

    //
    // If LocalHost is the first element in the array, switch it to be the second.
    // The first element is supposed to be the "default" interface on the machine,
    // which should really always be en0.
    if ((sNumIPAddrs > 1) && (::strcmp(sIPAddrInfoArray[0].fIPAddrStr.Ptr, "127.0.0.1") == 0))
    {
        UInt32 tempIP = sIPAddrInfoArray[1].fIPAddr;
        sIPAddrInfoArray[1].fIPAddr = sIPAddrInfoArray[0].fIPAddr;
        sIPAddrInfoArray[0].fIPAddr = tempIP;
        StrPtrLen tempIPStr(sIPAddrInfoArray[1].fIPAddrStr);
        sIPAddrInfoArray[1].fIPAddrStr = sIPAddrInfoArray[0].fIPAddrStr;
        sIPAddrInfoArray[0].fIPAddrStr = tempIPStr;
        StrPtrLen tempDNSStr(sIPAddrInfoArray[1].fDNSNameStr);
        sIPAddrInfoArray[1].fDNSNameStr = sIPAddrInfoArray[0].fDNSNameStr;
        sIPAddrInfoArray[0].fDNSNameStr = tempDNSStr;
    }
}

bool SocketUtils::IsMulticastIPAddr(UInt32 inAddress)
{
    return ((inAddress>>8) & 0x00f00000) == 0x00e00000; //  multicast addresses == "class D" == 0xExxxxxxx == 1,1,1,0,<28 bits>
}

bool SocketUtils::IsLocalIPAddr(UInt32 inAddress)
{
    for (UInt32 x = 0; x < sNumIPAddrs; x++)
        if (sIPAddrInfoArray[x].fIPAddr == inAddress)
            return true;
    return false;
}

void SocketUtils::ConvertAddrToString(const struct in_addr& theAddr, StrPtrLen* ioStr)
{
    sMutex.Lock();
    char* addr = inet_ntoa(theAddr);
    ::strcpy(ioStr->Ptr, addr);
    ioStr->Len = ::strlen(ioStr->Ptr);
    sMutex.Unlock();
}

UInt32 SocketUtils::ConvertStringToAddr(const char* inAddrStr)
{
    if (inAddrStr == NULL)
        return 0;
        
    return ntohl(::inet_addr(inAddrStr));
}

