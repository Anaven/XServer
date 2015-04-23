#ifndef __MSG_EVENT__
#define __MSG_EVENT__

#include "Queue.h"


class MsgEvent
{
    public:
       
    	MsgEvent(int theMsg, void* inData = NULL)
			: fMsg(theMsg), fResult(-1), fFinish(false), fData(inData), fElem(this) {};
    	~MsgEvent() {};

		void SetEvent(UInt32 inToDo, void* inData) { fMsg = inToDo; fData = inData; };

		enum
		{
			kKill = 0,
		};
		
		UInt32	fMsg;
		UInt32	fResult;
		bool	fFinish;
		void*	fData;
        QueueElem fElem;
};


class MsgEventReleaser
{
    public:

        MsgEventReleaser(MsgEvent* inMsgEvent) : fMsgEvent(inMsgEvent) {}
        ~MsgEventReleaser() { delete fMsgEvent; }
        
    private:

        MsgEvent* fMsgEvent;
};

#endif

