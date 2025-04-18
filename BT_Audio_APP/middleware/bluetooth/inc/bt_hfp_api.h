/**
 *******************************************************************************
 * @file    bt_hfp_api.h
 * @author  Halley
 * @version V1.0.1
 * @date    27-Apr-2016
 * @brief   HFP related api
 *******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

/**
* @addtogroup Bluetooth
* @{
* @defgroup bt_hfp_api bt_hfp_api.h
* @{
*/

#ifndef __BT_HFP_API_H__
#define __BT_HFP_API_H__

#include "type.h"

typedef enum{
	BT_STACK_EVENT_HFP_NONE = 0x00,			//0
	BT_STACK_EVENT_HFP_CONNECTED,
	BT_STACK_EVENT_HFP_DISCONNECTED,
	BT_STACK_EVENT_HFP_SCO_CONNECTED,
	BT_STACK_EVENT_HFP_SCO_STREAM_START,
	BT_STACK_EVENT_HFP_SCO_STREAM_PAUSE,	//5
	BT_STACK_EVENT_HFP_SCO_DISCONNECTED,
	BT_STACK_EVENT_HFP_SCO_DATA_RECEIVED,
	BT_STACK_EVENT_HFP_SCO_DATA_SENT,
	BT_STACK_EVENT_HFP_CALL_CONNECTED,
	BT_STACK_EVENT_HFP_CALL_DISCONNECTED,	//10
	BT_STACK_EVENT_HFP_CALLSETUP_NONE,
	BT_STACK_EVENT_HFP_CALLSETUP_IN,
	BT_STACK_EVENT_HFP_CALLSETUP_OUT,
	BT_STACK_EVENT_HFP_CALLSETUP_ALERT,
	BT_STACK_EVENT_HFP_RING,				//15
	BT_STACK_EVENT_HFP_CALLER_ID_NOTIFY,
	BT_STACK_EVENT_HFP_CURRENT_CALLS,
	BT_STACK_EVENT_HFP_CALL_WAIT_NOTIFY,
	BT_STACK_EVENT_HFP_SPEAKER_VOLUME,
	BT_STACK_EVENT_HFP_BATTERY,				//20
	BT_STACK_EVENT_HFP_SIGNAL,
	BT_STACK_EVENT_HFP_ROAM,
	BT_STACK_EVENT_HFP_VOICE_RECOGNITION,
	BT_STACK_EVENT_HFP_IN_BAND_RING,
	BT_STACK_EVENT_HFP_CODEC_TYPE,			//25
	BT_STACK_EVENT_HFP_MANUFACTORY_INFO,
	BT_STACK_EVENT_HFP_DEVICE_TYPE,
	BT_STACK_EVENT_HFP_UNSOLICITED_DATA,
	BT_STACK_EVENT_HFP_INDICATE_INFO,
	BT_STACK_EVENT_HFP_DEVICE_TIME,
	BT_STACK_EVENT_HFP_SERVICE,		/*service avaliability*/
}BT_HFP_CALLBACK_EVENT;

/**
 * The audio data format received from HFP of Bluetooth stack.
 *
 * If it supports WBS, two types of audio format can be selected
 * i). PCM means - The mSBC data in SCO has been decoded in the stack.
 * ii).mSBC means - The mSBC data in SCO has not been decoded yet.
 * 
 * If it doesn't support WBS, only PCM data can be received by application
 *
 */
typedef uint8_t	HfpAudioDataFormat;

#define HFP_AUDIO_DATA_PCM			1	/*!< HFP audio data is decoded PCM data*/

#define HFP_AUDIO_DATA_mSBC			2	/*!< HFP audio data is undecoded mSBC data*/


typedef struct _cbCallListParms
{

    /* Index of the call on the audio gateway (1 based). */
    uint8_t              index;

    /* 0 - Mobile Originated, 1 = Mobile Terminated */
    uint8_t              dir;

    /* Call state (see HfCallState). */
    uint8_t				state;

    /* Call mode (see HfCallMode). */
    uint8_t				mode;

    /* Phone number of the call */
    const char			*number;

}cbCallListParms;

typedef struct _cbCallIndicateParms
{
    uint8_t              indicator;

    uint8_t              value;
}cbCallIndicateParms;



typedef struct _BT_HFP_CALLBACK_PARAMS
{
	uint8_t						index;
	uint16_t					paramsLen;
	bool						status;
	uint16_t					errorCode;

	union
	{
		uint8_t					*bd_addr;
		
		const char				*hfpPhoneNumber;
		uint8_t					hfpVolGain;
		uint8_t					hfpBattery;
		uint8_t					hfpSignal;
		bool					hfpRoam;				/*roaming indicator*/
		bool					hfpVoiceRec;
		bool					hfpInBandRing;
		uint8_t 				*hfpUnsolicitedData;
		const char				*hfpRemoteManufactory;
		const char				*hfpRemoteDeviceType;
		const char				*hfpRemoteDeviceTime;
		uint8_t 				*scoReceivedData;
		uint8_t 				*scoSentData;
		HfpAudioDataFormat		scoCodecType;
		cbCallListParms			callListParms;
		cbCallIndicateParms		indicateParms;
		uint8_t					hfpService;				/*service avaliability*/
	}params;
}BT_HFP_CALLBACK_PARAMS;

typedef void (*BTHfpCallbackFunc)(BT_HFP_CALLBACK_EVENT event, BT_HFP_CALLBACK_PARAMS * param);

void BtHfpCallback(BT_HFP_CALLBACK_EVENT event, BT_HFP_CALLBACK_PARAMS * param);

typedef struct _HfpAppFeatures
{
	bool				wbsSupport;
	HfpAudioDataFormat	hfpAudioDataFormat;
	BTHfpCallbackFunc	hfpAppCallback;
}HfpAppFeatures;

/********************************************************************
 * @brief	User Command: HFP connect command.
 * @param	index: 0 or 1
 * @param	addr: The address of bt device to connect.
 * @return	True for the command implement successful
 * @Note	If the device has been connected with handsfree profile,
 * 			the event BT_STACK_EVENT_HFP_CONNECTED will be received in the callback.
 * 			Otherwise BT_STACK_EVENT_PAGE_TIMEOUT will be received.
 *******************************************************************/
signed char HfpConnect(uint8_t index, uint8_t * addr);


/********************************************************************
 * @brief	User Command: HFP disconnect command.
 * @param	index: 0 or 1
 * @return	True for the command implement successful
 * @Note	If the device has been disconnected.
 * 			the event BT_STACK_EVENT_HFP_DISCONNECTED will be received in the callback.
 *******************************************************************/
bool HfpDisconnect(uint8_t index);

/********************************************************************
 * @brief  User Command: Answer a incoming call.
 * @param  index: 0 or 1
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpAnswerCall(uint8_t index);


/** Indicates that the code should release all held calls, or set the User Determined User Busy (UDUB) indication for a waiting call.  
 */
#define HF_HOLD_RELEASE_HELD_CALLS   0

/** Indicates that the code should release all active calls (if any exist) and accepts the other (held or waiting) call.  
 *
 *  If a call index is specified, the code should release the specific call.  
 */
#define HF_HOLD_RELEASE_ACTIVE_CALLS 1

/** Indicates that the code should place all active calls (if any exist) on hold and accepts the other (held or waiting) call.  
 *
 *  If a call index is specified, the code should put all calls on hold except the specified call.  
 */
#define HF_HOLD_HOLD_ACTIVE_CALLS    2

/** Indicates that the code should add a held call to the conversation.  
 */
#define HF_HOLD_ADD_HELD_CALL        3

/** Indicates that the code should connects the two calls and disconnect the Audio Gateway from both calls.  
 *  In other words, the code should perform an Explicit Call Transfer.  
 */
#define HF_HOLD_CALL_TRANSFER        4
/* End of HfHoldAction */ 

/********************************************************************
 * @brief  Hold a call (三方通话)
 * @param  index: 0 or 1
 * @param  callstate: 三方通话参数
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpCallHold(uint8_t index, int callState);

/********************************************************************
 * @brief  User Command: Dial a number.
 * @param  index: 0 or 1
 * @param  number: The string of the number to be dialed
 * @param  numberLen: The string length of the number
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpDialNumber(uint8_t index, uint8_t * number, uint8_t numberLen);

/********************************************************************
 * @brief  User Command: Redial the last outgoing number.
 * @param  index: 0 or 1
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpRedialNumber(uint8_t index);

/********************************************************************
 * @brief	User Command: Terminates an active call,
 *			cancels an outgoing call or rejects an incoming call.
 * @param	index: 0 or 1
 * @return	True for the command implement successful
 * @Note
 *******************************************************************/
bool HfpHangup(uint8_t index);

/********************************************************************
 * @brief  User Command: Get current calls in Audio Gateway(Phone)
 * @param  index: 0 or 1
 * @return True for the command implement successful 
 * @Note The event BT_STACK_EVENT_HFP_CURRENT_CALLS with phone number will be received
 *******************************************************************/
bool HfpGetCurrentCalls(uint8_t index);

/********************************************************************
 * @brief  User Command: Generate a DTMF code to the network.
 * @param  index: 0 or 1
 * @param  dtmfTone: A single ASCII character in the set 0-9, #, *, A-D.
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpDtmf(uint8_t index, uint8_t dtmfTone);

/********************************************************************
 * @brief  User Command: Enables or disables voice recognition on the Audio Gateway
 * @param  index: 0 or 1
 * @param  enable: Set to TRUE if voice recognition is enabled, and FALSE if it is disabled.
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpVoiceRecognition(uint8_t index, bool enable);

/********************************************************************
 * @brief  User Command: Disables noise reduction and echo canceling in Audio Gateway(Phone)
 * @param  index: 0 or 1
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpDisableNREC(uint8_t index);

/********************************************************************
 * @brief  User Command: Gets the current speaker volume of the Hands-Free device
 * @param  index: 0 or 1
 * @param  gain: The current gain level (0-15).
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpSpeakerVolume(uint8_t index, uint8_t gain);

/********************************************************************
 * @brief  User Command: Gets the manufactory information of remote device
 * @param  index: 0 or 1
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpGetRemoteDeviceManufactory(uint8_t index);

/********************************************************************
 * @brief	User Command: Gets the type of remote device
 * @param	index: 0 or 1
 * @param	gain: The current gain level (0-15).
 * @return	True for the command implement successful
 * @Note
 *******************************************************************/
bool HfpGetRemoteDeviceType(uint8_t index);

/********************************************************************
 * @brief	Set apple battery status
 * @param	index: 0 or 1
 * @param	BatteryLevel 0-9
 * @param	DockingState 0-1
 * @return	NONE
 * @note
 *******************************************************************/
void HfpSetBatteryState(uint8_t index, uint8_t BatteryLevel, uint8_t DockingState);

/********************************************************************
 * @brief  User Command: Sends any AT command..
 * @param  atCommand: The string of the atCommand to be sent
 * @param  commandLen The string length of the atCommand
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpSendAtCommand(const char * atCommand, uint8_t commandLen);

/********************************************************************
 * @brief  User Command: Disconnect audio connection (SCO) between Hands Free device and Audio Gateway
 * @param  index: 0 or 1
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpAudioDisconnect(uint8_t index);

/********************************************************************
 * @brief  User Command: Create an audio connection (SCO) between Hands Free device and Audio Gateway
 * @param  index: 0 or 1
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpAudioConnect(uint8_t index);

/********************************************************************
 * @brief  User Command: Transfer audio between Hands Free device and Audio Gateway
 * @param  index: 0 or 1
 * @return True for the command implement successful 
 * @Note
 *******************************************************************/
bool HfpAudioTransfer(uint8_t index);

/********************************************************************
 * @brief User Command: Send hfp audio data
 * @param index: 0 or 1
 * @param data: The audio data to be sent
 * @param dataLen: The length of audio data to be sent
 * @return TRUE means the data has been insert to send
 *			queue The event BT_STACK_EVENT_HFP_SCO_DATA_SENT
 *			will be received when the audio data has been 
 *			sent successful
 * @Note
 *******************************************************************/
bool HfpSendScoData(uint8_t index, uint8_t * data, uint16_t dataLen);

/********************************************************************
 * @brief  Set HFP enable/disable WBS function
 * @param  enable: TRUE for enable WBS function
 * @return NONE
 * @Note
 *******************************************************************/
void WbsSupport(bool enable);

/********************************************************************
 * @brief  Get HFP enable/disable WBS function
 * @param  None
 * @return TRUE for enable WBS function
 * @Note
 *******************************************************************/
bool GetWbsSupport(void);

/********************************************************************
 * @brief  Get HFP sco type
 * @param  index: 0 or 1
 * @return 0=cvsd; 1=msbc
 * @Note
 *******************************************************************/
uint8_t GetHfpScoAudioCodecType(uint8_t index);

/********************************************************************
 * @brief  Get HFP call state (区分呼入/呼出状态)
 * @param  None
 * @return 0=none
 *         1=incoming
 *         2=outgoing
 * @Note
 *******************************************************************/
uint8_t GetHfpCallState(void);

/********************************************************************
 * @brief  hfp app init
 * @param  HfpAppFeatures
 * @return 
 * @Note
 *******************************************************************/
bool HfpAppInit(HfpAppFeatures *params);

/********************************************************************
 * @brief  hfp get indicator status
 * @param  index
 * @return 
 * @Note
 *******************************************************************/
bool HfpGetIndicatorStatus(uint8_t index);

/********************************************************************
 * @brief  hfp get remote device time
 * @param  index
 * @return 
 * @Note
 *******************************************************************/
bool HfpGetRemoteDevTime(uint8_t index);


#endif

