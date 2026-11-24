#include "type.h"
#include "debug.h"
#include "bt_manager.h"
#include "bt_app_a2dp_deal.h"
#include "bt_interface.h"

#if	BT_SOURCE_SUPPORT
static void SetSourceA2dpState(uint8_t index, BT_A2DP_STATE state);

extern uint8_t A2dpSourceDisconnect(uint8_t index);
extern uint32_t gSbcEncoderReinit;//初始化标志
extern void A2DP_StreamSendSbcDataBufClear(uint8_t index);

void BtA2dpSourceCallback(uint8_t index, BT_A2DP_CALLBACK_EVENT event, BT_A2DP_CALLBACK_PARAMS * param)
{
	switch(event)
	{
		case BT_STACK_EVENT_A2DP_CONNECTED:
		{
			A2DP_StreamSendSbcDataBufClear(index);
			SetSourceA2dpState(index, BT_A2DP_STATE_CONNECTED);

			if((param->params.bd_addr)[0] || (param->params.bd_addr)[1] || (param->params.bd_addr)[2]
				|| (param->params.bd_addr)[3] || (param->params.bd_addr)[4] || (param->params.bd_addr)[5])
			{
				memcpy(GetBtManager()->remoteAddr, param->params.bd_addr, 6);
			}

			SetBtConnectedProfile(index, BT_CONNECTED_A2DP_FLAG);
			BtLinkStateConnect(0, index);
			APP_DBG("A2dp source connected[%d] %2x %2x %2x %2x %2x %2x\n",index,param->params.bd_addr[0],param->params.bd_addr[1],param->params.bd_addr[2],
						param->params.bd_addr[3] ,param->params.bd_addr[4],param->params.bd_addr[5]);


			 //if(gSwitchSourceAndSink==A2DP_SET_SINK)//Bt Source立刻发起AVRCP连接可能导致AVRCP连接不上
				BtAvrcpConnect(index, GetBtManager()->remoteAddr);

			extern void BtSourceCheckConn(uint8_t *Addr,uint32_t timeout);
			BtSourceCheckConn(GetBtManager()->remoteAddr,500);

			BT_Source_Connect(index,GetBtManager()->remoteAddr);
			{
				gSbcEncoderReinit = 1;
			}
		}
		break;

		case BT_STACK_EVENT_A2DP_DISCONNECTED:
		{
			SetSourceA2dpState(index, BT_A2DP_STATE_NONE);
			SetBtDisconnectProfile(index, BT_CONNECTED_A2DP_FLAG);// bkd need change
			BtLinkStateDisconnect(index);
			APP_DBG("A2dp source disconnect\n");
		}
		break;

		case BT_STACK_EVENT_A2DP_CONNECT_TIMEOUT:
		{
			APP_DBG("A2dp source connect timeout\n");
			A2dpSourceDisconnect(index);
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_START:
		{
			SetSourceA2dpState(index, BT_A2DP_STATE_STREAMING);
			APP_DBG("A2dp source streaming...\n");
//			AudioCoreSinkEnable(AUDIO_BT_SOURCE_SINK_NUM);
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_SUSPEND:
		{
			APP_DBG("A2dp source suspend\n");
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_DATA_IND:
		{
			APP_DBG("A2dp source data\n");
		}
		break;

		case BT_STACK_EVENT_A2DP_STREAM_DATA_TYPE:
		{
			APP_DBG("A2dp stream type ");
		}
		break;

		default:
		break;
	}
}


static void SetSourceA2dpState(uint8_t index, BT_A2DP_STATE state)
{
	if(index >= BT_SRC_LINK_DEV_NUM) return 0;
	GetBtManager()->a2dpSourceState[index] = state;
}

BT_A2DP_STATE GetSourceA2dpState(uint8_t index)
{
	if(index >= BT_SRC_LINK_DEV_NUM) return 0;
	return GetBtManager()->a2dpSourceState[index];
}
#endif
