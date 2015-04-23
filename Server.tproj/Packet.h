#ifndef __PACKET_H__
#define __PACKET_H__

#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

/*
class Packet
{
	public:

        // packet to device or daclient
    	enum 
        { 
        	kDaRegisterRequest 		= 0x03, 
        	kDevRegisterRequest     = 0x04, 
        	kEnrollReply    		= 0x05, 
        	kHeartWork	            = 0x06,
        	kSoundDa2Dev           	= 0x0e,   
        	kSoundDev2Da           	= 0x0f,   
        	kVideoDev2Da	        = 0x10,
        	kComDa2Dev           	= 0x20,
        	kComDev2Da           	= 0x21,  
        	kSoundDaRegister     	= 0x22, 
        	kSoundDevRegister     	= 0x23, 
            
        	kCMETVideoRegister      = 0x24, 
        	kDeviceVideoRegister    = 0x25,
        	kAdjustSendSpeed       	= 0x26,
        	kImageDaChangeFormate  	= 0x30,
        	
			kDownloadApp			= 0x31,
			kDownloadReply			= 0x32,

			kDeviceState			= 0x33,
			kDeviceLog				= 0x34,

			kInstallApp				= 0x36,
			kInstallAppReply		= 0x37,
			kRunApp					= 0x38,
			kRunAppReply			= 0x39,
			kUninstallApp			= 0x40,
			kUninstallAppReply		= 0x41,

			kAllowObserveOrNot			= 0x42,
			kInput					= 0x43,
			kSynchronizeTime		= 0x44,
			kCleanAllApp			= 0x45,
			kAllowObserveReply	= 0x46,
			kCleanAllAppReply		= 0x47,

			kChangeVideoFormate		= 0x49,
        };
                
    	enum 
		{ 
			kMessage = 0x61, 
			kAudio = 0x71, 
			kVideo = 0x81 
		};
		
    	enum { kMessagePacketHeaderSize = 3, 	kMessagePacketCtrlSize = 1 	};
    	enum { kAudioPacketHeaderSize = 3, 		kAudioPacketCtrlSize = 1 	};
    	enum { kVideoPacketHeaderSize = 5, 		kVideoPacketCtrlSize = 1 	};

    	enum // kReply
        {    
        	kRegisterSuccess           	= 0x00,
        	kObserverSaturation         = 0x01,
        	kSessionConflict	        = 0x02,
        	kUnknownError	            = 0x03,
        	kSessionNotFound	        = 0x04,
        	kObserverRegisterSuccess   	= 0x05,
        	kExitNormal	                = 0x06,
        	kSelfConflict	            = 0x07,
        	kAuthorizeFailed	        = 0x08,
        	kTimeout	                = 0x09,
        	kInternalError	            = 0x0A,
        	kNotLoginPortal	            = 0x0B,
        	kAuthorityError	            = 0x0C,
        	kExpire	                    = 0x0D,
        	kOtherConflict	            = 0x0E,
        	kBeWatched					= 0x0F,
        	kNotBeWatched				= 0x10
        };

		enum
		{
        	kAllowToBeWatched			= 0x11,
        	kDisallowToBeWathed			= 0x12
		};

		enum
		{
			kH264				= 0x40,
			kFLV				= 0x41,
			kJPEG				= 0x42,
		};
};

*/

#endif

