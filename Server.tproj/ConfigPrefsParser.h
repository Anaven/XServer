#ifndef __CONFIG_PREF_PARSER__
#define __CONFIG_PREF_PARSER__

#include "Headers.h"

#define CONFIG_PREFS_PARSER_DEBUG 1

enum 
{ 
	kIPSize             = 32,
	kServerIDSize	    = 32,
	kFilePathSize	    = 1024,
}; 

class ConfigParameter
{
	public: 
		
		enum
		{
			kParameterNameSize 	= 64,
			kValueSize			= 256,
		};

		char 	fName[kParameterNameSize];
		char 	fValue[kValueSize];
};

class ConfigPrefsParser
{
	public:

    	ConfigPrefsParser(const char *theFilePath);
        ~ConfigPrefsParser() {};

    	bool DoesFileExist();
    	int Parse();

    	UInt32 	GetMessagePacketSize()		{ return *(UInt32*)this->GetValue("message_packet_size");		};
		UInt32	GetMaxMessagePackets()		{ return *(UInt32*)this->GetValue("max_message_packets");		};

    	UInt32 	GetAudioPacketSize()      	{ return *(UInt32*)this->GetValue("audio_packet_size");			};
    	UInt32 	GetMaxAudioPackets()      	{ return *(UInt32*)this->GetValue("max_audio_packets");			};

    	UInt32 	GetVideoPacketSize()     	{ return *(UInt32*)this->GetValue("video_packet_size");			};
    	UInt32 	GetMaxVideoPackets() 		{ return *(UInt32*)this->GetValue("max_video_packets");			};

    	UInt32 	GetShortTaskThreadsNum()	{ return *(UInt32*)this->GetValue("short_task_threads_num");	};
    	UInt32 	GetBlockingThreadsNum()   	{ return *(UInt32*)this->GetValue("blocking_threads_num");		};
    	UInt32 	GetProcessNum()          	{ return *(UInt32*)this->GetValue("processors_num");       		};

    	SInt32 	GetMaxConnections()  	{ return *(SInt32*)this->GetValue("max_connections");	};
    	UInt32	GetMaxObserver() 		{ return *(UInt32*)this->GetValue("max_oberver");   	};
        
    	UInt16 	GetMessagePort()     	{ return *(UInt16*)this->GetValue("message_port"); 		};
    	UInt16 	GetAudioPort()     		{ return *(UInt16*)this->GetValue("audio_port");       	};
    	UInt16 	GetVideoPort()  		{ return *(UInt16*)this->GetValue("video_port");      	};

    	UInt32 	GetKeepAlive()       	{ return *(UInt32*)this->GetValue("keep_alive");      	};
		UInt32	GetSynchronizeTime()	{ return *(UInt32*)this->GetValue("synchronize_time");	};
        
    	UInt16 	GetWebPort()         	{ return *(UInt16*)this->GetValue("web_port");       	};
    	char*	GetWebIP()      		{ return (char*)this->GetValue("web_ip");             	};
    	char*	GetServerID()     		{ return (char*)this->GetValue("server_id");            };
		
		char*	GetLogPath()     		{ return (char*)this->GetValue("log_path");            };

	private:

		enum
		{
			kConfigParameterListSize 	= 100,
		};
		
		void 	SetValue(char* theLine, char* theName, char* theType, UInt32 theIndex);
		void* 	GetValue(const char* theName);

		int		fFile;
		char 	fFilePath[kFilePathSize];
		
		UInt32	fTotalParameterNum;
		ConfigParameter fConfigParameterList[kConfigParameterListSize];
};

#endif
