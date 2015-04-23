#ifndef __SESSION_LISTENER_SOCKET__
#define __SESSION_LISTENER_SOCKET__

#include "TCPListenerSocket.h"


class MessageListenerSocket: public TCPListenerSocket
{
    public:
    
        MessageListenerSocket() {}
        virtual ~MessageListenerSocket() {}
        
        //sole job of this object is to implement this function
        virtual Task*   GetSessionTask(TCPSocket** outSocket);

        //check whether the Listener should be idling
    	bool OverMaxConnections(UInt32 buffer);
};

#endif

