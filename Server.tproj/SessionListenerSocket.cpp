#include "SessionListenerSocket.h"
#include "NetStream.h"
#include "XServer.h"


Task* MessageListenerSocket::GetSessionTask(TCPSocket** outSocket)
{
    Assert(outSocket != NULL);
    NetStream* theTask = NEW NetStream;
    *outSocket = theTask->GetSocket();  // out socket is not attached to a unix socket yet.
    
    if (this->OverMaxConnections(0))
        this->SlowDown();
    else
        this->RunNormal();        
    
    return theTask;
}


bool MessageListenerSocket::OverMaxConnections(UInt32 buffer)
{
    XServer *theServer = XServer::GetServer();
    SInt32 maxConns = theServer->GetMaxConnections();
    bool overLimit = false;

    /*
    if (maxConns > -1) // limit connections
    { 
        maxConns += buffer;
        if (theServer->GetNumMessageSession() > (UInt32)maxConns) 
        {
            overLimit = true;
        }
    } 
    */
    return overLimit;
}

