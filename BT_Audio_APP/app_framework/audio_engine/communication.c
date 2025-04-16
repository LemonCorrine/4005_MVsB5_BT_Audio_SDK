#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <nds32_intrinsic.h>
#include "uarts.h"
#include "dma.h"
#include "uarts_interface.h"
#include "timeout.h"
#include "debug.h"
#include "app_config.h"
#include "i2s.h"
#include "i2s_interface.h"
#include "clk.h"
#include "ctrlvars.h"
#include "audio_adc.h"
#include "dac.h"
#include "spdif.h"
#include "uarts_interface.h"
#include "audio_effect_library.h"
#include "rtos_api.h"
#include "roboeffect_api.h"
#include "communication.h"
#include "audio_core_api.h"
#include "roboeffect_prot.h"
#include "ctrlvars.h"
#include "main_task.h"
#include "user_effect_parameter.h"
#ifdef BT_PROFILE_BQB_ENABLE
#include "bt_bqb_test.h"
#endif

#ifdef CFG_FUNC_USBDEBUG_EN
bool hid_tx_buf_is_used = 0;
#endif
#ifdef CFG_FUNC_AUDIO_EFFECT_ONLINE_TUNING_EN

#define  CTL_DATA_SIZE   2

uint8_t  cbuf[8];
uint8_t  tx_buf[256]     = {0xa5, 0x5a, 0x00, 0x00,};

const char AudioLibVer[] = AUDIO_EFFECT_LIBRARY_VERSION;
const char RoboeffectLibVer[] = ROBOEFFECT_LIB_VER;

extern uint8_t  hid_tx_buf[];
extern uint32_t SysemMipsPercent;
extern int16_t cpu_core_clk;
extern uint16_t sizeof_effect_property_for_display(void);

extern uint8_t GetEffectModeParaCount(void);
extern uint8_t GetCurSameEffectModeID(uint8_t index);
extern AUDIOEFFECT_EFFECT_PARA * GetCurSameEffectModeAudioPara(uint8_t index);
extern uint8_t GetCurEffectModeNum(void);
extern void AudioEffect_effect_enable(uint8_t effect_addr, uint8_t enable);

static const uint32_t SupportSampleRateList[]={
	8000,
	11025,
	12000,
	16000,
	22050,
	24000,
	32000,
	44100,
	48000,
	//i2s//////////////////
	88200,
	96000,
	176400,
	192000,
};

static int32_t comm_ret_sample_rate_enum(uint32_t samplerate)
{
	int i;
	for(i=0; i<(sizeof(SupportSampleRateList)/sizeof(uint32_t)); i++)
	{
		if(SupportSampleRateList[i] == samplerate)
			return i;
	}
	return 7;//default 44100
}

void HIDUsb_Rx(uint8_t *buf,uint16_t len)
{
#ifdef BT_PROFILE_BQB_ENABLE
	UsbLoadBQBCmd(len, buf);
#endif
	roboeffect_prot_parse_big_block(buf, len);
}

void Communication_Effect_Send(uint8_t *buf, uint32_t len)
{
#ifdef CFG_COMMUNICATION_BY_USB
#ifdef CFG_FUNC_USBDEBUG_EN
	hid_tx_buf_is_used = 1;
#endif
	memcpy(hid_tx_buf, buf, 256);
#endif
#ifdef CFG_COMMUNICATION_BY_UART
	DMA_CircularDataPut(CFG_FUNC_COMMUNICATION_TX_DMA_PORT, buf, len);
#endif
}

bool roboeffect_effect_update_params_entrance(uint8_t addr, uint8_t *buf, uint32_t len)
{
//	int i;

	tx_buf[0] = 0xA5;
	tx_buf[1] = 0x5A;
	tx_buf[2] = addr;

	if(len == 0)//require parameters
	{
        const int16_t *params;
        if(((params = roboeffect_get_effect_parameter(AudioEffect.context_memory, addr, 0xff)) != NULL)
        		&& (gCtrlVars.AutoRefresh != AutoRefresh_ALL_PARA) && (gCtrlVars.AutoRefresh != AutoRefresh_ALL_EFFECTS_PARA))
        {
            int16_t len = roboeffect_get_effect_parameter_count(AudioEffect.context_memory, addr);
            if(len >= 0)//= 0 when only enable/disable existed
            {
                int16_t *pp = (int16_t*)&tx_buf[5];//out parameter 0 is enable/disable

                tx_buf[3] = (len + 1) * 2 + 1;
                tx_buf[4] = 0xff;

                *(pp++) = roboeffect_get_effect_status(AudioEffect.context_memory, addr);//on/off
                memcpy(pp, params, len * 2);//parameters
                pp += len;

                tx_buf[4 + (len + 1) * 2 + 1] = 0x16;
                Communication_Effect_Send(tx_buf,  tx_buf[3] + 5);
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
        	if(gCtrlVars.AutoRefresh != AutoRefresh_ALL_EFFECTS_PARA)
        	{
        		gCtrlVars.AutoRefresh = AutoRefresh_ALL_PARA;
        	}
    		extern void Communication_Effect_0x02(void);
    		Communication_Effect_0x02();
            return FALSE;
        }
	}
	else//set parameters
	{
        if(buf[0] == 0xff)//all parameters
        {
			int16_t enable = buf[1];//[1] is enable/disable
			uint8_t *params = (uint8_t *)&buf[3];//[3] is parameters beginning

			EffectValidParamUnit unit = AudioEffect_GetUserEffectValidParam(AudioEffect.user_effect_parameters);
			uint8_t *effectParams =  unit.params_first_address;
			uint16_t data_len = unit.data_len;
			uint8_t len = 0;

			while(data_len)
			{
				if(*effectParams == addr)
				{
					effectParams ++;
					len = *effectParams;
					effectParams++;
					*effectParams = enable;
					effectParams++;
					if(len > 1)
					{
						memcpy(effectParams, params, len - 1);
					}
					break;
				}
				else
				{
					effectParams++;
					len = *effectParams;
					effectParams += (len + 1);
					if(data_len > (len + 1))
						data_len -= (len + 1);
					else
						return FALSE;
				}
			}
			if (addr == AudioEffect.effect_count || AudioEffect.reinit_done)
			{
                AudioEffect.reinit_done = 0;
				AudioEffect_effect_enable(addr,enable);
			}
        }
        else//one parameter
        {
            if(buf[0] == 0)//only enable/disable
            {
            	AudioEffect_effect_enable(addr,buf[1]);
            }
            else//other parameter, only one parameter configurated
            {
				int16_t *params = (int16_t *)&buf[1];//[1] is parameter
                int16_t index = buf[0] - 1;//[0] - 1 is parameter index

//                if(roboeffect_get_effect_status(AudioEffect.context_memory, addr))
                {
                	roboeffect_set_effect_parameter(AudioEffect.context_memory, addr, index, params);

                	if(len == 3)
                	{
                		AudioEffect_update_local_params(addr, index, params, len - 1);
                	}
                	else
                	{
                		AudioEffect_update_local_block_params(addr);
                	}
                }
            }
        }

        Communication_Effect_Send(&addr, 1);

	}

	return TRUE;
}

uint8_t graph_cnt = 0;
static uint8_t package_id = 0;
void roboeffect_effect_enquiry_stream(uint8_t *buf, uint32_t len)
{
	uint8_t *temp;
	uint16_t total_len = 0;
	static int32_t remain_len;
	static const char *char_pp;

    temp = tx_buf;

	if(buf[0] == V3_PACKAGE_TYPE_PARAM)//enquiry params
	{
		temp[0] = 0xA5;
		temp[1] = 0x5A;
		temp[2] = 0x80;
		temp[3] = 10;//len including EOM
		temp[4] = 0x00;//package id
		temp[5] = 0xFF;
		temp[6] = 0x00;
		temp[7] = V3_PACKAGE_TYPE_PARAM;//type

		temp[8] = 0x01;//graph count

		*(uint16_t *)&temp[9] = AudioCoreFrameSizeGet(DefaultNet);
		*(uint16_t *)&temp[11] = comm_ret_sample_rate_enum(AudioEffect.cur_effect_para->user_effect_list->sample_rate);
		
		temp[13] = 0x01;//EOM
		temp[14] = 0x16;
		Communication_Effect_Send(temp, temp[3] + 5);

		package_id = 0;
	}
	else if(buf[0] == V3_PACKAGE_TYPE_LIB || buf[0] == V3_PACKAGE_TYPE_STREAM)//enquiry stream
	{
		if(buf[0] == V3_PACKAGE_TYPE_LIB)
		{
			// total_len = sizeof_effect_property_for_display();
			// printf("lib start: %d\n", total_len);
		}
		else if(buf[0] == V3_PACKAGE_TYPE_STREAM)
		{
			// total_len = get_user_effects_script_len();
			// printf("script start: %d\n", total_len);
		}

		//[package_id, 0xFF, 0x00, type, len_H, len_L, data...data, EOM]
		temp[0] = 0xA5;
		temp[1] = 0x5A;
		temp[2] = 0x80;
		//len later
		temp[4] = package_id;

		if(package_id == 0)//first package
		{
			uint16_t len;
			if(buf[0] == V3_PACKAGE_TYPE_LIB)
			{
				char_pp = (const char*)effect_property_for_display;
				total_len = sizeof_effect_property_for_display();
				// printf("first display\n");
			}
			else if(buf[0] == V3_PACKAGE_TYPE_STREAM)
			{
				char_pp = (const char*)AudioEffect.cur_effect_para->user_effects_script;
				total_len = AudioEffect.user_effects_script_len;
			}

			remain_len = total_len;//init remain_len
			if(remain_len >= STREAM_CLIPS_LEN)
				len = STREAM_CLIPS_LEN;
			else
				len = remain_len;
			
			temp[3] = len + 7;
			temp[5] = 0xFF;
			temp[6] = 0x00;
			if(buf[0] == V3_PACKAGE_TYPE_LIB)
				temp[7] = V3_PACKAGE_TYPE_LIB;//type
			else if(buf[0] == V3_PACKAGE_TYPE_STREAM)
				temp[7] = V3_PACKAGE_TYPE_STREAM;//type

			*((uint16_t*)(&temp[8])) = total_len;

			memcpy(&temp[10], char_pp, len);

			remain_len -= len;
			char_pp += len;

			if(remain_len > 0)
			{
				temp[10 + len] = 0x00;//EOM
				package_id ++;
			}
			else
			{
				temp[10 + len] = 0x01;//EOM
				package_id = 0;
			}
			
			temp[10 + len + 1] = 0x16;

			Communication_Effect_Send(temp, len + 12);
		}
		else
		{
			uint16_t len;
			if(remain_len >= STREAM_CLIPS_LEN)
				len = STREAM_CLIPS_LEN;
			else
				len = remain_len;

			temp[3] = len + 2;
			memcpy(&temp[5], char_pp, len);

			remain_len -= len;
			char_pp += len;

			if(remain_len > 0)
			{
				temp[5 + len] = 0x00;//EOM
				package_id ++;
			}
			else
			{
				temp[5 + len] = 0x01;//EOM
				package_id = 0;

				//switch to next graph
			}
			
			temp[5 + len + 1] = 0x16;

			Communication_Effect_Send(temp, len + 7);
		}
	}
	else if (buf[0] == V3_PACKAGE_TYPE_EFFECT_NAME)
	{
		temp[0] = 0xA5;
		temp[1] = 0x5A;
		temp[2] = 0x80;
		//len later
		temp[4] = package_id;

		if(package_id == 0)//first package
		{

			uint16_t len;
			total_len = GetEffectNameTotalLen() + GetEffectModeParaCount() - 1;

			remain_len = total_len;//init remain_len
			if(remain_len >= STREAM_CLIPS_LEN)
				len = STREAM_CLIPS_LEN;
			else
				len = remain_len;

			temp[3] = len + 5;
			temp[5] = 0xFF;
			temp[6] = 0x00;
			temp[7] = V3_PACKAGE_TYPE_EFFECT_NAME;//type

			GetEffectNameCurPackageforCommunication(0 , &temp[8]);

			remain_len -= len;

			if(remain_len > 0)
			{
				temp[8 + len] = 0x00;//EOM
				package_id ++;
			}
			else
			{
				temp[8 + len] = 0x01;//EOM
				package_id = 0;
			}
			temp[8 + len + 1] = 0x16;

			Communication_Effect_Send(temp, temp[3] + 5);
		}
		else
		{
			uint16_t len;
			total_len = GetEffectNameTotalLen() + GetEffectModeParaCount() - 1;
			if(remain_len >= STREAM_CLIPS_LEN)
				len = STREAM_CLIPS_LEN;
			else
				len = remain_len;

			temp[3] = len + 3;
			GetEffectNameCurPackageforCommunication(total_len - remain_len , &temp[6]);

			remain_len -= len;

			if(remain_len > 0)
			{
				temp[6 + len] = 0x00;//EOM
				package_id ++;
			}
			else
			{
				temp[6 + len] = 0x01;//EOM
				package_id = 0;
			}
			temp[6 + len + 1] = 0x16;

			Communication_Effect_Send(temp, temp[3] + 5);
		}
	}
	else if (buf[0] == V3_PACKAGE_TYPE_EFFECT_ID)
	{
		temp[0] = 0xA5;
		temp[1] = 0x5A;
		temp[2] = 0x80;
		temp[3] = 7;//len including EOM
		temp[4] = 0x00;//package id
		temp[5] = 0xFF;
		temp[6] = 0x00;
		temp[7] = V3_PACKAGE_TYPE_EFFECT_ID;//type

		temp[8] = 0x0;//reserved
		temp[9] = GetCurEffectModeNum();
//		APP_DBG("effect mode: %d\n", temp[9]);

		temp[10] = 0x01;//EOM
		temp[11] = 0x16;
		Communication_Effect_Send(temp, temp[3] + 5);

		package_id = 0;
	}
}

void Communication_Effect_0x00(void)
{
	char *pp;
    memset(tx_buf, 0, sizeof(tx_buf));
	tx_buf[0]  = 0xa5;
	tx_buf[1]  = 0x5a;
	tx_buf[2]  = 0x00;
	tx_buf[3]  = 0x0E;
	tx_buf[4]  = 0x42;//41=B1; 42=B5

	tx_buf[5]  = CFG_SDK_MAJOR_VERSION;
	tx_buf[6]  = CFG_SDK_MINOR_VERSION;
	tx_buf[7]  = CFG_SDK_PATCH_VERSION;

	pp = (char *)AudioLibVer;

	tx_buf[8] = atoi(pp);
	while(pp-AudioLibVer < strlen(AudioLibVer) && *pp != '.'){pp++;}
	tx_buf[9] = atoi(++pp);
	while(pp-AudioLibVer < strlen(AudioLibVer) && *pp != '.'){pp++;}
	tx_buf[10] = atoi(++pp);

	pp = (char *)RoboeffectLibVer;

	tx_buf[11] = atoi(pp);
	while(pp-RoboeffectLibVer < strlen(RoboeffectLibVer) && *pp != '.'){pp++;}
	tx_buf[12] = atoi(++pp);
	while(pp-RoboeffectLibVer < strlen(RoboeffectLibVer) && *pp != '.'){pp++;}
	tx_buf[13] = atoi(++pp);

	tx_buf[14] = 0x02;
	tx_buf[15] = 'B';
	tx_buf[16] = '5';
	tx_buf[17] = 0x01;
	//-----------------------------------//
	tx_buf[18] = 0x16;
	Communication_Effect_Send(tx_buf, 19);
}

void Communication_Effect_0x01(uint8_t *buf, uint32_t len)
{
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x01;
		tx_buf[3]  = 1+8*2;
		tx_buf[4]  = 0xff;
		tx_buf[10]  = 0x1;//System Sample Rate Enable
		tx_buf[11] = comm_ret_sample_rate_enum(AudioCore.SampleRate[DefaultNet]);;
		tx_buf[13]  = 0x1;//SDK MCLK为全局
		memcpy(&tx_buf[19], &AudioCore.FrameSize[DefaultNet], 2);//注意需要确认使用哪个变量Sam mask
		tx_buf[5 + 8*2] = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 8*2);
	}
	//else
	//{
		//do nothing
	//}
}

void Communication_Effect_0x02(void)///systme ram
{
	uint16_t UsedRamSize = (CFG_CHIP_RAM_SIZE - osPortRemainMem())/1024;;
	uint16_t CpuMaxFreq = Clock_CoreClockFreqGet() / 1000000;
	uint16_t cpu_mips = (uint16_t)(((10000 - SysemMipsPercent) * (Clock_CoreClockFreqGet()/1000000)) / 10000);
	uint16_t CpuMaxRamSize = CFG_CHIP_RAM_SIZE/1024;//
	uint16_t cpu_core_clk = roboeffect_get_error_code(AudioEffect.context_memory);

	memset(tx_buf, 0, sizeof(tx_buf));

	tx_buf[0]  = 0xa5;
	tx_buf[1]  = 0x5a;
	tx_buf[2]  = 0x02;
	tx_buf[3]  = 1 +  1 + 2*5 ;
	tx_buf[4]  = 0xff;

	memcpy(&tx_buf[5], &UsedRamSize, 2);
	memcpy(&tx_buf[7], &cpu_mips, 2);
	//是否自动刷新数据，后续需要改进 Sam mask
	tx_buf[9] = gCtrlVars.AutoRefresh;
	gCtrlVars.AutoRefresh = AutoRefresh_NONE;

	memcpy(&tx_buf[10], &CpuMaxFreq, 2);
	memcpy(&tx_buf[12], &CpuMaxRamSize, 2);
	memcpy(&tx_buf[14], &cpu_core_clk, 2);
	tx_buf[16] = 0x16;
	Communication_Effect_Send(tx_buf, tx_buf[3] + 5);
}

void Comm_PGA0_0x03(uint8_t * buf)
{
	uint16_t TmpData;

	switch(buf[0])///ADC0 PGA
	{
		case 0:///line1 Left show?
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_show = !!TmpData;
			break;
		case 1://line1 Right show?
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_show = !!TmpData;
			break;
		case 2:///line1 Left en?
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_en = !!TmpData;
#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN1)
			AudioLineSelSet(ANA_INPUT_CH_LINEIN1);
#else
			AudioLineSelSet(ANA_INPUT_CH_LINEIN2);
#endif
			break;
		case 3://line1 Right en?
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_en = !!TmpData;
#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN1)
			AudioLineSelSet(ANA_INPUT_CH_LINEIN1);
#else
			AudioLineSelSet(ANA_INPUT_CH_LINEIN2);
#endif
			break;
		case 4://line1 Left gain
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_gain = TmpData > 31? 31 : TmpData;
#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN1)
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT, 31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_gain);
#else
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN2_LEFT, 31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_gain);
#endif
			break;
		case 5://line1 Right gain
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_gain = TmpData > 31? 31 : TmpData;
#if (LINEIN_INPUT_CHANNEL == ANA_INPUT_CH_LINEIN1)
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT, 31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_gain);
#else
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN2_RIGHT, 31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_gain);
#endif
			break;
		default:
			break;
	}
}

void Communication_Effect_0x03(uint8_t *buf, uint32_t len)////ADC0 PGA
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]   = 0xa5;
		tx_buf[1]   = 0x5a;
		tx_buf[2]   = 0x03;//cmd
		tx_buf[3]   = 1+6*2;///1 + len * sizeof(int16)
		tx_buf[4]   = 0xff;///all paramgs,
		memcpy(&tx_buf[5],  &gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_show, 2);
		memcpy(&tx_buf[7],  &gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_show, 2);
		memcpy(&tx_buf[9],  &gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_en, 2);
		memcpy(&tx_buf[11], &gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_en, 2);
		memcpy(&tx_buf[13], &gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_gain, 2);
		memcpy(&tx_buf[15], &gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_gain, 2);

		tx_buf[5 + 6*2]  = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 6*2);
	}
	else
	{
		switch(buf[0])///ADC0 PGA
		{
			case 0xff:
				buf++;
				for(i = 0; i < 19; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_PGA0_0x03(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_PGA0_0x03(buf);
				break;
		}
	}
}

void Comm_ADC0_0x04(uint8_t * buf)
{
	uint16_t TmpData;
	bool LeftEnable;
	bool RightEnable;
	bool LeftMute;
	bool RightMute;

	switch(buf[0])///adc0 digital channel en
	{
		case 0://ADC0 en?
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0DigitalCt.adc_channel_en = TmpData > 3? 3 : TmpData;
			LeftEnable = gCtrlVars.HwCt.ADC0DigitalCt.adc_channel_en & 0x01;
			RightEnable = (gCtrlVars.HwCt.ADC0DigitalCt.adc_channel_en >> 1) & 0x01;
			AudioADC_LREnable(ADC0_MODULE, LeftEnable, RightEnable);
			break;
		case 1:///ADC0 mute select?
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0DigitalCt.adc_mute = TmpData > 3 ? 3 : TmpData;
			LeftMute = gCtrlVars.HwCt.ADC0DigitalCt.adc_mute & 0x01;
			RightMute = (gCtrlVars.HwCt.ADC0DigitalCt.adc_mute >> 1) & 0x01;
			AudioADC_DigitalMute(ADC0_MODULE, LeftMute, RightMute);
			break;
		case 2://adc0 dig vol left
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0DigitalCt.adc_dig_l_vol = TmpData > 0x3fff ? 0x3fff : TmpData;
			AudioADC_VolSetChannel(ADC0_MODULE, CHANNEL_LEFT,(uint16_t)gCtrlVars.HwCt.ADC0DigitalCt.adc_dig_l_vol);
			break;
		case 3://adc0 dig vol right
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0DigitalCt.adc_dig_r_vol = TmpData > 0x3fff? 0x3fff : TmpData;
			AudioADC_VolSetChannel(ADC0_MODULE, CHANNEL_RIGHT, (uint16_t)gCtrlVars.HwCt.ADC0DigitalCt.adc_dig_r_vol);
			break;
		case 4://adc0 sample rate
			break;
		case 5://adc0 LR swap
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC0DigitalCt.adc_lr_swap = !!TmpData;//不能直接传参数？需要通过if/else来实现
			AudioADC_ChannelSwap(ADC0_MODULE, gCtrlVars.HwCt.ADC0DigitalCt.adc_lr_swap);
			break;
		case 6://adc0 hight pass
			memcpy(&gCtrlVars.HwCt.ADC0DigitalCt.adc_hpc, &buf[1], 2);
			gCtrlVars.HwCt.ADC0DigitalCt.adc_hpc = TmpData > 2? 2: TmpData;
			AudioADC_HighPassFilterConfig(ADC0_MODULE, HPCList[gCtrlVars.HwCt.ADC0DigitalCt.adc_hpc]);
			break;
		case 7://adc0 fade time
			break;
		case 8://adc0 mclk src
#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
			memcpy(&gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source, &buf[1], 2);
	        gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source = gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source > 2 ? (gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source - 1):gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source;
	        Clock_AudioMclkSel(AUDIO_ADC0, gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source);
#endif
		    break;
		case 9://hpc0 en
			break;
		 default:
		   break;
	}
}

void Communication_Effect_0x04(uint8_t *buf, uint32_t len)////ADC0 DIGITAL
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x04;
		tx_buf[3]  = 1+10*2;///1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
		memcpy(&tx_buf[5], &gCtrlVars.HwCt.ADC0DigitalCt.adc_channel_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.HwCt.ADC0DigitalCt.adc_mute, 2);
		memcpy(&tx_buf[9], &gCtrlVars.HwCt.ADC0DigitalCt.adc_dig_l_vol, 2);
		memcpy(&tx_buf[11], &gCtrlVars.HwCt.ADC0DigitalCt.adc_dig_r_vol, 2);
		memcpy(&tx_buf[13], &gCtrlVars.sample_rate_index, 2);//定义一个全局结构体，用于上传PC
		memcpy(&tx_buf[15], &gCtrlVars.HwCt.ADC0DigitalCt.adc_lr_swap,2);
		memcpy(&tx_buf[17], &gCtrlVars.HwCt.ADC0DigitalCt.adc_hpc, 2);
		tx_buf[19] = 5;
		memcpy(&tx_buf[21], &gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source, 2);
		memcpy(&tx_buf[23], &gCtrlVars.HwCt.ADC0DigitalCt.adc_dc_blocker_en, 2);////adc0 hpc en
		tx_buf[25] = 0x16;
		Communication_Effect_Send(tx_buf, 26);///25+3*4+1
	}
	else
	{
		switch(buf[0])///ADC0 PGA
		{
			case 0xff:
				buf++;
				for(i = 0; i < 10; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_ADC0_0x04(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_ADC0_0x04(buf);
				break;
		}
	}
}

void Comm_PGA1_0x06(uint8_t * buf)
{
	uint16_t TmpData;

	switch(buf[0])//ADC1 PGA
	{
		case 0:///mic show?
		    memcpy(&TmpData, &buf[1], 2);
		    gCtrlVars.HwCt.ADC1PGACt.pga_mic_show = !!TmpData;
			break;
//		case 1:////mic mode ?
//		    memcpy(&TmpData, &buf[1], 2);
//		    gCtrlVars.HwCt.ADC1PGACt.pga_mic_mode = !!TmpData;
//			break;
		case 2:////mic en ?
		    memcpy(&TmpData, &buf[1], 2);
		    gCtrlVars.HwCt.ADC1PGACt.pga_mic_enable = !!TmpData;
			AudioLine3MicSelect();
			break;
		case 3://mic gain
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1PGACt.pga_mic_gain = TmpData > 31 ? 31 : TmpData;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, MIC_LEFT, 31-gCtrlVars.HwCt.ADC1PGACt.pga_mic_gain);
			break;
		default:
			break;
	}
}

void Communication_Effect_0x06(uint8_t *buf, uint32_t len)////ADC1 PGA
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x06;//cmd
		tx_buf[3]  = 1 + 4*2;///1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;///all paramgs
		memcpy(&tx_buf[5], &gCtrlVars.HwCt.ADC1PGACt.pga_mic_show, 2);
		memcpy(&tx_buf[7], &gCtrlVars.HwCt.ADC1PGACt.pga_mic_mode, 2);
		memcpy(&tx_buf[9], &gCtrlVars.HwCt.ADC1PGACt.pga_mic_enable, 2);
		memcpy(&tx_buf[11], &gCtrlVars.HwCt.ADC1PGACt.pga_mic_gain, 2);
		tx_buf[5 + 4*2]  = 0x16;
		Communication_Effect_Send(tx_buf, 6 + 4*2);
	}
	else
	{
		switch(buf[0])///adc1 digital set
		{
			case 0xff:
				buf++;
				for(i = 0; i < 9; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_PGA1_0x06(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_PGA1_0x06(buf);
				break;
		}
	}
}

void Comm_ADC1_0x07(uint8_t * buf)
{
	uint16_t TmpData;
	bool LeftEnable;
	bool RightEnable;
	bool LeftMute;
	bool RightMute;

	switch(buf[0])///adc1 digital channel en
	{
		case 0://ADC1 en?
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1DigitalCt.adc_channel_en = TmpData > 3? 3 : TmpData;
			LeftEnable = gCtrlVars.HwCt.ADC1DigitalCt.adc_channel_en & 01;
			RightEnable = (gCtrlVars.HwCt.ADC1DigitalCt.adc_channel_en >> 1) & 01;
			AudioADC_LREnable(ADC1_MODULE, LeftEnable, RightEnable);
			break;
		case 1:///ADC1 mute select?
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1DigitalCt.adc_mute = TmpData > 3? 3 : TmpData;
			LeftMute = gCtrlVars.HwCt.ADC1DigitalCt.adc_mute & 0x01;
			RightMute = (gCtrlVars.HwCt.ADC1DigitalCt.adc_mute >> 1) & 0x01;
			AudioADC_DigitalMute(ADC1_MODULE, LeftMute, RightMute);
			break;
		case 2://adc1 dig vol left
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1DigitalCt.adc_dig_l_vol = TmpData > 0x3fff? 0x3fff : TmpData;
			AudioADC_VolSetChannel(ADC1_MODULE, CHANNEL_LEFT,(uint16_t)gCtrlVars.HwCt.ADC1DigitalCt.adc_dig_l_vol);
			break;
		case 3://adc1 dig vol right
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1DigitalCt.adc_dig_r_vol = TmpData > 0x3fff? 0x3fff : TmpData;
			AudioADC_VolSetChannel(ADC1_MODULE, CHANNEL_RIGHT, (uint16_t)gCtrlVars.HwCt.ADC1DigitalCt.adc_dig_r_vol);
			break;
		case 4://adc1 sample rate
			break;
		case 5://adc1 LR swap
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1DigitalCt.adc_lr_swap = !!TmpData;
			AudioADC_ChannelSwap(ADC1_MODULE, gCtrlVars.HwCt.ADC1DigitalCt.adc_lr_swap);
			break;
		case 6://adc1 hight pass
			memcpy(&gCtrlVars.HwCt.ADC1DigitalCt.adc_hpc, &buf[1], 2);
			gCtrlVars.HwCt.ADC1DigitalCt.adc_hpc = TmpData > 2? 2: TmpData;
			AudioADC_HighPassFilterConfig(ADC1_MODULE, HPCList[gCtrlVars.HwCt.ADC1DigitalCt.adc_hpc]);
			break;
		case 7://adc1 fade time
			break;
		case 8://adc1 mclk src
#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
			memcpy(&gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source, &buf[1], 2);
	        gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source = gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source > 2 ? (gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source - 1):gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source;
	        Clock_AudioMclkSel(AUDIO_ADC1, gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source);
#endif
		    break;
		case 9://hpc1 en
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1DigitalCt.adc_dc_blocker_en = !!TmpData;
			AudioADC_HighPassFilterSet(ADC1_MODULE, gCtrlVars.HwCt.ADC1DigitalCt.adc_dc_blocker_en);
			break;
		 default:
		   break;
	}
}

void Communication_Effect_0x07(uint8_t *buf, uint32_t len)///ADC1 DIGITAL
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x07;
		tx_buf[3]  = 1+10*2;///1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
		memcpy(&tx_buf[5], &gCtrlVars.HwCt.ADC1DigitalCt.adc_channel_en, 2);
		memcpy(&tx_buf[7], &gCtrlVars.HwCt.ADC1DigitalCt.adc_mute, 2);
		memcpy(&tx_buf[9], &gCtrlVars.HwCt.ADC1DigitalCt.adc_dig_l_vol, 2);
		memcpy(&tx_buf[11], &gCtrlVars.HwCt.ADC1DigitalCt.adc_dig_r_vol, 2);
		memcpy(&tx_buf[13], &gCtrlVars.sample_rate_index, 2);
		memcpy(&tx_buf[15],  &gCtrlVars.HwCt.ADC1DigitalCt.adc_lr_swap,2);
		memcpy(&tx_buf[17], &gCtrlVars.HwCt.ADC1DigitalCt.adc_hpc, 2);
		tx_buf[19] = 5;
		memcpy(&tx_buf[21], &gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source, 2);
		memcpy(&tx_buf[23], &gCtrlVars.HwCt.ADC1DigitalCt.adc_dc_blocker_en, 2);////adc0 hpc en
		tx_buf[25] = 0x16;
		Communication_Effect_Send(tx_buf, 26);///25+3*4+1
	}
	else
	{
		switch(buf[0])///ADC0 PGA
		{
			case 0xff:
				buf++;
				for(i = 0; i < 10; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_ADC1_0x07(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_ADC1_0x07(buf);
				break;
		}
	}
}

void Comm_AGC1_0x08(uint8_t * buf)
{
	uint16_t TmpData;
	bool	AgcLeftEn;
	bool	AgcRightEn;
	switch(buf[0])//ADC1 AGC
	{
		case 0://AGC {buf[1]=0 dis} {buf[1]=1 left en} {buf[1]=2 right en} {buf[1]=3 left+right en}
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_mode = TmpData > 3? 3 : TmpData;
			AgcLeftEn = gCtrlVars.HwCt.ADC1AGCCt.adc_agc_mode & 0x01;
			AgcRightEn = (gCtrlVars.HwCt.ADC1AGCCt.adc_agc_mode >> 1) & 0x01;
			AudioADC_AGCChannelSel(ADC1_MODULE, AgcLeftEn, AgcRightEn);
			break;
		case 1://MAX level
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_max_level = TmpData > 31? 31 : TmpData;
			AudioADC_AGCMaxLevel(ADC1_MODULE,(uint8_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_max_level);
			break;
		case 2://target level
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_target_level = TmpData > 255? 255 : TmpData;
			AudioADC_AGCTargetLevel(ADC1_MODULE,(uint8_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_target_level);
			break;
		case 3://max gain
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_max_gain =  TmpData > 31? 31 : TmpData;
			TmpData = 31 - gCtrlVars.HwCt.ADC1AGCCt.adc_agc_max_gain;
			AudioADC_AGCMaxGain(ADC1_MODULE,(uint8_t)TmpData);
			break;
		case 4://min gain
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_min_gain = TmpData > 31? 31 : TmpData;
			TmpData = 31 - gCtrlVars.HwCt.ADC1AGCCt.adc_agc_min_gain;
			AudioADC_AGCMinGain(ADC1_MODULE,(uint8_t)TmpData);
			break;
		case 5://gain offset
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_gainoffset = TmpData > 15? 15 : TmpData;
			AudioADC_AGCGainOffset(ADC1_MODULE,(uint8_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_gainoffset);
			break;
		case 6://fram time
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_fram_time = TmpData > 4096? 4096 : TmpData;
			AudioADC_AGCFrameTime(ADC1_MODULE,(uint16_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_fram_time);
			break;
		case 7://hold time
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_hold_time = TmpData > 4096? 4096 : TmpData;
			AudioADC_AGCHoldTime(ADC1_MODULE,(uint32_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_hold_time);
			break;
		case 8://attack time
			memcpy(&TmpData, &buf[1], 2);
			TmpData = TmpData > 4096? 4096 : TmpData;
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_attack_time = TmpData;
			AudioADC_AGCAttackStepTime(ADC1_MODULE,(uint16_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_attack_time);
			break;
		case 9://dacay time
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_decay_time = TmpData > 4096? 4096 : TmpData;
			AudioADC_AGCDecayStepTime(ADC1_MODULE,(uint16_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_decay_time);
			break;
		case 10://nosie gain en
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_gate_en = TmpData > 1? 1 : TmpData;
			AudioADC_AGCNoiseGateEnable(ADC1_MODULE,(bool)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_gate_en);
			break;
		case 11://nosie thershold
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_threshold = TmpData > 31? 31 : TmpData;
			AudioADC_AGCNoiseThreshold(ADC1_MODULE,(uint8_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_threshold);
			break;
		case 12://nosie gate mode
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_gate_mode = TmpData > 1? 1 : TmpData;
			AudioADC_AGCNoiseGateMode(ADC1_MODULE,(uint8_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_gate_mode);
			break;
		case 13://nosie gate hold time
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_time = TmpData > 4096? 4096 : TmpData;
			AudioADC_AGCNoiseHoldTime(ADC1_MODULE,(uint8_t)gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_time);
			break;
		default:
			break;
	}
}

void Communication_Effect_0x08(uint8_t *buf, uint32_t len)////ADC1 AGC
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x08;
		tx_buf[3]  = 1 + 14*2;///1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
        memcpy(&tx_buf[5], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_mode, 2);
		memcpy(&tx_buf[7], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_max_level, 2);
		memcpy(&tx_buf[9], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_target_level, 2);
		memcpy(&tx_buf[11], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_max_gain, 2);
		memcpy(&tx_buf[13], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_min_gain, 2);
		memcpy(&tx_buf[15], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_gainoffset, 2);
		memcpy(&tx_buf[17], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_fram_time, 2);
		memcpy(&tx_buf[19], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_hold_time, 2);
		memcpy(&tx_buf[21], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_attack_time, 2);
		memcpy(&tx_buf[23], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_decay_time, 2);
		memcpy(&tx_buf[25], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_gate_en, 2);
		memcpy(&tx_buf[27], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_threshold, 2);
		memcpy(&tx_buf[29], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_gate_mode, 2);
		memcpy(&tx_buf[31], &gCtrlVars.HwCt.ADC1AGCCt.adc_agc_noise_time, 2);
		tx_buf[33] = 0x16;
		Communication_Effect_Send(tx_buf, 34);
	}
	else
	{
		switch(buf[0])///ADC1 AGC
		{
			case 0xff:
				buf++;
				for(i = 0; i < 14; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_AGC1_0x08(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_AGC1_0x08(buf);
				break;
		}
	}
}

void Comm_DAC0_0x09(uint8_t * buf)
{
    uint16_t TmpData;
	bool leftMute;
	bool rightMute;

	switch(buf[0])////DAC0 set
	{
		case 0://DAC0 en
			break;
        case 1://dac0 sample rate 0~8
			break;
        case 2:///dac0 mute
        	memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.DAC0Ct.dac_dig_mute = TmpData > 3? 3 : TmpData;
			leftMute = gCtrlVars.HwCt.DAC0Ct.dac_dig_mute & 0x01;
			rightMute = (gCtrlVars.HwCt.DAC0Ct.dac_dig_mute >> 1) & 0x01;
			AudioDAC_DigitalMute(DAC0, leftMute, rightMute);
			break;
		case 3:////dac0 L volume
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.DAC0Ct.dac_dig_l_vol = TmpData > 0x3fff? 0x3fff : TmpData;
//			gCtrlVars.HwCt.DAC0Ct.dac_dig_r_vol = gCtrlVars.HwCt.DAC0Ct.dac_dig_l_vol;
			AudioDAC_VolSet(DAC0, gCtrlVars.HwCt.DAC0Ct.dac_dig_l_vol, gCtrlVars.HwCt.DAC0Ct.dac_dig_r_vol);
			break;
		case 4:////dac0 R volume
			memcpy(&TmpData, &buf[1], 2);
			gCtrlVars.HwCt.DAC0Ct.dac_dig_r_vol = TmpData > 0x3fff? 0x3fff : TmpData;
//			gCtrlVars.HwCt.DAC0Ct.dac_dig_l_vol = gCtrlVars.HwCt.DAC0Ct.dac_dig_r_vol;
			AudioDAC_VolSet(DAC0, gCtrlVars.HwCt.DAC0Ct.dac_dig_l_vol, gCtrlVars.HwCt.DAC0Ct.dac_dig_r_vol);
			break;
        case 5:///DAC0 dither
            memcpy(&TmpData, &buf[1], 2);
            gCtrlVars.HwCt.DAC0Ct.dac_dither = !!TmpData;
			if(gCtrlVars.HwCt.DAC0Ct.dac_dither) AudioDAC_DitherEnable(DAC0);
			else                      AudioDAC_DitherDisable(DAC0);
            break;
		case 6:///dac0 scramble
            memcpy(&TmpData, &buf[1], 2);
            gCtrlVars.HwCt.DAC0Ct.dac_scramble = TmpData > 3? 3 : TmpData;
			if(gCtrlVars.HwCt.DAC0Ct.dac_scramble == 0)
			{
				AudioDAC_ScrambleDisable(DAC0);
			}
			else
			{
				AudioDAC_ScrambleEnable(DAC0);
				AudioDAC_ScrambleModeSet(DAC0,(SCRAMBLE_MODULE)gCtrlVars.HwCt.DAC0Ct.dac_scramble - 1);
			}
            break;
        case 7:///dac0 stere mode
            memcpy(&TmpData, &buf[1], 2);
            gCtrlVars.HwCt.DAC0Ct.dac_out_mode = TmpData > 3? 3 : TmpData;
#ifndef CFG_AUDIO_WIDTH_24BIT
			AudioDAC_DoutModeSet(DAC0, (DOUT_MODE)gCtrlVars.HwCt.DAC0Ct.dac_out_mode, WIDTH_16_BIT);//注意DAC位宽，一般情况下使用不到
#else
            AudioDAC_DoutModeSet(DAC0, (DOUT_MODE)gCtrlVars.HwCt.DAC0Ct.dac_out_mode, WIDTH_24_BIT_2);//注意DAC位宽，一般情况下使用不到
#endif
            break;
        case 13:///dac0 mclk source
#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
			memcpy(&gCtrlVars.HwCt.DAC0Ct.dac_mclk_source, &buf[1], 2);
	        gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source = gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source > 2 ? (gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source - 1):gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source;
	        Clock_AudioMclkSel(DAC0, gCtrlVars.HwCt.DAC0Ct.dac_mclk_source);
#endif
		default:
			break;
	}
}

void Communication_Effect_0x09(uint8_t *buf, uint32_t len)///DAC0
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x09;
		tx_buf[3]  = 1 + 14*2;///1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
 	    tx_buf[5] = 3;		//L+R enable
		memcpy(&tx_buf[7], &gCtrlVars.sample_rate_index, 2);
        memcpy(&tx_buf[9], &gCtrlVars.HwCt.DAC0Ct.dac_dig_mute, 2);
		memcpy(&tx_buf[11], &gCtrlVars.HwCt.DAC0Ct.dac_dig_l_vol, 2);
		memcpy(&tx_buf[13], &gCtrlVars.HwCt.DAC0Ct.dac_dig_r_vol, 2);
		memcpy(&tx_buf[15], &gCtrlVars.HwCt.DAC0Ct.dac_dither, 2);
		memcpy(&tx_buf[17], &gCtrlVars.HwCt.DAC0Ct.dac_scramble, 2);
		memcpy(&tx_buf[19], &gCtrlVars.HwCt.DAC0Ct.dac_out_mode, 2);
        tx_buf[27] = 5;
		memcpy(&tx_buf[31], &gCtrlVars.HwCt.DAC0Ct.dac_mclk_source, 2);
		tx_buf[33] = 0x16;
		Communication_Effect_Send(tx_buf,34);
	}
	else
	{
		switch(buf[0])///dac0
		{
			case 0xff:
				buf++;
				for(i = 0; i < 14; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_DAC0_0x09(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_DAC0_0x09(buf);
				break;
		}
	}
}

void Comm_I2S0_0x0B(uint8_t * buf)
{

	switch(buf[0])//I2S0
	{
		case 3:///I2S0 mclk source
#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
			memcpy(&gCtrlVars.HwCt.I2S0Ct.i2s_mclk_source, &buf[1], 2);
	        gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source = gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source > 2 ? (gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source - 1):gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source;
			Clock_AudioMclkSel(AUDIO_I2S0, gCtrlVars.HwCt.I2S0Ct.i2s_mclk_source);
#endif
			break;
		default:
			break;
	}
}
void Communication_Effect_0x0B(uint8_t *buf, uint32_t len)//I2S0
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
#if defined(CFG_APP_I2SIN_MODE_EN) || defined(CFG_RES_AUDIO_I2SOUT_EN) || defined(CFG_RES_AUDIO_I2S_MIX_IN_EN) || defined(CFG_RES_AUDIO_I2S_MIX_OUT_EN)
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x0b;
		tx_buf[3]  = 1 + 11*2;///1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
#if (defined(CFG_RES_AUDIO_I2SOUT_EN) && (CFG_RES_I2S_MODULE == 0)) || (defined(CFG_RES_AUDIO_I2S_MIX_OUT_EN) && (CFG_RES_MIX_I2S_MODULE == 0))
		tx_buf[5] = 1;
#else
		tx_buf[5] = 0;
#endif
#if (defined(CFG_APP_I2SIN_MODE_EN) && (CFG_RES_I2S_MODULE == 0)) || (defined(CFG_RES_AUDIO_I2S_MIX_IN_EN) && (CFG_RES_MIX_I2S_MODULE == 0))
		tx_buf[7] = 1;
#else
		tx_buf[7] = 0;
#endif
		tx_buf[9] = comm_ret_sample_rate_enum(I2S_SampleRateGet(I2S0_MODULE));
		tx_buf[11] = gCtrlVars.HwCt.I2S0Ct.i2s_mclk_source;
		tx_buf[13] = AudioI2S_MasterModeGet(I2S0_MODULE);
		tx_buf[15] = I2S_WordlengthGet(I2S0_MODULE);
		tx_buf[17] = 0;
		tx_buf[19] = 90;
		tx_buf[21] = 1;
		tx_buf[27] = 0x16;
#endif
		Communication_Effect_Send(tx_buf, tx_buf[3] + 5);
	}
	else
	{
		switch(buf[0])///I2S0
		{
			case 0xff:
				buf++;
				for(i = 0; i < 19; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_I2S0_0x0B(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_I2S0_0x0B(buf);
				break;
		}
	}
}

void Comm_I2S1_0x0C(uint8_t * buf)
{

	switch(buf[0])//I2S1
	{
		case 3:///I2S1 mclk source
#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
			memcpy(&gCtrlVars.HwCt.I2S1Ct.i2s_mclk_source, &buf[1], 2);
	        gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source = gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source > 2 ? (gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source - 1):gCtrlVars.HwCt.ADC0DigitalCt.adc_mclk_source;
			Clock_AudioMclkSel(AUDIO_I2S1, gCtrlVars.HwCt.I2S1Ct.i2s_mclk_source);
#endif
			break;
		default:
			break;
	}
}
void Communication_Effect_0x0C(uint8_t *buf, uint32_t len)//I2S1
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
#if defined(CFG_APP_I2SIN_MODE_EN) || defined(CFG_RES_AUDIO_I2SOUT_EN) || defined(CFG_RES_AUDIO_I2S_MIX_IN_EN) || defined(CFG_RES_AUDIO_I2S_MIX_OUT_EN)
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x0c;
		tx_buf[3]  = 1 + 11*2;///1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;
#if (defined(CFG_RES_AUDIO_I2SOUT_EN) && (CFG_RES_I2S_MODULE == 1)) || (defined(CFG_RES_AUDIO_I2S_MIX_OUT_EN) && (CFG_RES_MIX_I2S_MODULE == 1))
		tx_buf[5] = 1;
#else
		tx_buf[5] = 0;
#endif
#if (defined(CFG_APP_I2SIN_MODE_EN) && (CFG_RES_I2S_MODULE == 1)) || (defined(CFG_RES_AUDIO_I2S_MIX_IN_EN) && (CFG_RES_MIX_I2S_MODULE == 1))
		tx_buf[7] = 1;
#else
		tx_buf[7] = 0;
#endif
		tx_buf[9] = comm_ret_sample_rate_enum(I2S_SampleRateGet(I2S1_MODULE));
		tx_buf[11] = gCtrlVars.HwCt.I2S1Ct.i2s_mclk_source;
		tx_buf[13] = AudioI2S_MasterModeGet(I2S1_MODULE);
		tx_buf[15] = I2S_WordlengthGet(I2S1_MODULE);
		tx_buf[17] = 0;
		tx_buf[19] = 90;
		tx_buf[21] = 2;
		tx_buf[27] = 0x16;
#endif
		Communication_Effect_Send(tx_buf, tx_buf[3] + 5);
	}
	else
	{
		switch(buf[0])///I2S1
		{
			case 0xff:
				buf++;
				for(i = 0; i < 19; i++)
				{
					cbuf[0] = i;////id
					for(k = 0; k < CTL_DATA_SIZE; k++)
					{
						cbuf[ k + 1] = (uint8_t)buf[k];
					}
					Comm_I2S1_0x0C(&cbuf[0]);
					buf += 2;
				}
				break;
			default:
				Comm_I2S1_0x0C(buf);
				break;
		}
	}
}

void Communication_Effect_0x0D(uint8_t *buf, uint32_t len)//SPDIF
{
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
#if defined(CFG_APP_OPTICAL_MODE_EN) || defined(CFG_APP_COAXIAL_MODE_EN)
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x0d;
		tx_buf[3]  = 1 + 10*2;///1 + len * sizeof(int16)
		tx_buf[4]  = 0xff;

		tx_buf[5]  = 1;
		tx_buf[7]  = SPDIF_SampleRateGet(SPDIF0);
#ifdef CFG_APP_OPTICAL_MODE_EN
		tx_buf[9] = SPDIF0_OPTICAL_PORT_MODE - 1;
		tx_buf[11] = SPDIF0_OPTICAL_PORT_ANA_INPUT;
#endif
#ifdef CFG_APP_COAXIAL_MODE_EN
		tx_buf[9] = SPDIF0_COAXIAL_PORT_MODE - 1;
		tx_buf[11] = SPDIF0_COAXIAL_PORT_ANA_INPUT;
#endif
		tx_buf[13] = SPDIF_FlagStatusGet(SPDIF0, LOCK_FLAG_STATUS);
		tx_buf[15] = 0;
		tx_buf[17] = SPDIF_SampleRateGet(SPDIF1);
		tx_buf[19] = 0;
		tx_buf[21] = 0;
		tx_buf[23] = SPDIF_FlagStatusGet(SPDIF1, LOCK_FLAG_STATUS);
		tx_buf[24] = 0x16;
#endif
		Communication_Effect_Send(tx_buf,26);
	}
}

void Communication_Effect_0x80(uint8_t *buf, uint32_t len)
{
	if(len == 1)//enquiry stream
	{
		roboeffect_effect_enquiry_stream(buf, len);
	}
	else if (len == 2)
	{
		if (buf[0] == V3_PACKAGE_TYPE_EFFECT_ID)
		{
			AudioEffect.effect_mode_expected = GetCurSameEffectModeID(buf[1]);
//			APP_DBG("mode change:%d\n", AudioEffect.effect_mode_expected);
			MessageContext msgSend;
			msgSend.msgId = MSG_EFFECTMODE;
			MessageSend(GetMainMessageHandle(), &msgSend);
		}
	}
}

void Communication_Effect_0xfb(uint8_t *buf, uint32_t len)
{
	uint8_t num = buf[0], *addr = &(buf[1]), out_num = 0;
	uint32_t *ptr_size = (uint32_t *)&(tx_buf[5]);

	tx_buf[0] = 0xa5;
	tx_buf[1] = 0x5a;
	tx_buf[2] = 0xfb;

	for(int i = 0; i < num; i++)
	{
		uint32_t t_size;
		roboeffect_get_effect_size(AudioEffect.context_memory, addr[i], &t_size);
		// printf("-->0x%02X: %d\n", addr[i], t_size);
		if(t_size < 0) break;//not enough nodes to get
		*ptr_size = t_size;
		ptr_size++;
		*ptr_size = roboeffect_recommend_frame_size_upon_effect_change(AudioEffect.context_memory, AudioEffect.audioeffect_frame_size, addr[i], 1);
		ptr_size++;
		out_num++;
	}
	tx_buf[3] = 1 + out_num * 4 * 2;
	tx_buf[4] = out_num;
	tx_buf[tx_buf[3] + 4] = 0x16;

	// printf("send %d: ", tx_buf[3] + 5);
	// for(int i = 0; i<tx_buf[3] + 5; i++)
	// {
	// 	printf("%02X ", tx_buf[i]);
	// }
	// printf("\n");

	Communication_Effect_Send(tx_buf, tx_buf[3] + 5);
}

//32个字节
void Communication_Effect_0xfc(uint8_t *buf, uint8_t len)//user define tag
{
	memset(tx_buf, 0, sizeof(tx_buf));
	tx_buf[0]  = 0xa5;
	tx_buf[1]  = 0x5a;
	tx_buf[2]  = 0xfc;//control
	if(len == 0)
	{
		uint8_t temp[64] = {0};
#ifdef CFG_EFFECT_PARAM_IN_FLASH_EN
		if(AudioEffect.EffectFlashUseFlag)
		{
			strcpy((char *)temp,(char *)EFFECT_FLASH_MODE_NAME);
			strcat((char *)temp,(char *)AudioEffect.cur_effect_para->user_effect_name);
		}
		else
#endif
		{
			strcpy((char *)temp,(char *)AudioEffect.cur_effect_para->user_effect_name);
		}

		tx_buf[3]  = strlen((char *)temp);//len
		memcpy(&tx_buf[4], temp, tx_buf[3]);
		tx_buf[4+tx_buf[3]] = 0x16;
		Communication_Effect_Send(tx_buf, 4+tx_buf[3]+1);
	}
#ifdef CFG_EFFECT_PARAM_UPDATA_BY_ACPWORKBENCH
	else if(len == 1)
	{
		tx_buf[3]  = 1;
		tx_buf[4]  = buf[0];
		tx_buf[5]  = 0x16;
		Communication_Effect_Send(&tx_buf[2], 1);//返回控制字
		//EffectParamFlashIndex = tx_buf[4] - '0';
		//if(EffectParamFlashIndex >= 10)
		//{
		//	EffectParamFlashIndex = 0xFF;
		//}
	}
#endif
}

#ifdef CFG_EFFECT_PARAM_UPDATA_BY_ACPWORKBENCH
bool EffectParamFlashUpdataFlag = FALSE;
#endif

void Communication_Effect_0xfd(uint8_t *buf, uint8_t len)//user define tag
{
#ifdef CFG_EFFECT_PARAM_UPDATA_BY_ACPWORKBENCH
	memset(tx_buf, 0, sizeof(tx_buf));
	tx_buf[0]  = 0xa5;
	tx_buf[1]  = 0x5a;
	tx_buf[2]  = 0xfd;//control
	tx_buf[3]  = 0x16;
	Communication_Effect_Send(&tx_buf[2], 1);//返回控制字
	EffectParamFlashUpdataFlag = TRUE;
#endif
}

void Communication_Effect_0xff(uint8_t *buf, uint32_t len)
{
    uint32_t TmpData;
	memset(tx_buf, 0, sizeof(tx_buf));
	tx_buf[0] = 0xa5;
	tx_buf[1] = 0x5a;
	tx_buf[2] = 0xFF;
	tx_buf[3] = 3;
	if(len == 0)
	{
		tx_buf[4] = 0;//index0
		tx_buf[5] = CFG_COMMUNICATION_CRYPTO;
	}
	else
	{
		memcpy(&TmpData, &buf[0], 4);
		tx_buf[4] = 1;//index1
		if(CFG_COMMUNICATION_PASSWORD != TmpData)
		{
			tx_buf[5] = 0;///passwrod err
		}
		else
		{
		    tx_buf[5] = 1;///passwrod ok
		}
	}
	tx_buf[7] = 0x16;
	Communication_Effect_Send(tx_buf, 8);
}

#ifdef CFG_COMMUNICATION_BY_UART
#define ROBO_UART_BUF_MAX_LEN 512
uint8_t robo_uart_recv_buf[ROBO_UART_BUF_MAX_LEN] = {0};
MemHandle uart_recv_handle;

static uint8_t uart_tx_buf[512];
static uint8_t uart_rx_buf[512];
static uint8_t s_rx_buf[512];
static uint8_t uart_buf[4096];
void uart_data_init(void)
{

	if(GET_DEBUG_GPIO_PORT(CFG_FUNC_COMMUNICATION_RX_PORT) == DEBUG_GPIO_PORT_A)
		GPIO_PortAModeSet(GET_DEBUG_GPIO_PIN(CFG_FUNC_COMMUNICATION_RX_PORT), GET_DEBUG_GPIO_MODE(CFG_FUNC_COMMUNICATION_RX_PORT));
	else
		GPIO_PortBModeSet(GET_DEBUG_GPIO_PIN(CFG_FUNC_COMMUNICATION_RX_PORT), GET_DEBUG_GPIO_MODE(CFG_FUNC_COMMUNICATION_RX_PORT));

	if(GET_DEBUG_GPIO_PORT(CFG_FUNC_COMMUNICATION_TX_PORT) == DEBUG_GPIO_PORT_A)
		GPIO_PortAModeSet(GET_DEBUG_GPIO_PIN(CFG_FUNC_COMMUNICATION_TX_PORT), GET_DEBUG_GPIO_MODE(CFG_FUNC_COMMUNICATION_TX_PORT));
	else
		GPIO_PortBModeSet(GET_DEBUG_GPIO_PIN(CFG_FUNC_COMMUNICATION_TX_PORT), GET_DEBUG_GPIO_MODE(CFG_FUNC_COMMUNICATION_TX_PORT));

	UARTS_Init(GET_DEBUG_GPIO_UARTPORT(CFG_FUNC_COMMUNICATION_TX_PORT), CFG_FUNC_COMMUNICATION_UART_BAUDRATE, 8, 0, 1);

	UART_IOCtl(GET_DEBUG_GPIO_UARTPORT(CFG_FUNC_COMMUNICATION_TX_PORT), UART_IOCTL_DMA_TX_EN, 1);
	UART_IOCtl(GET_DEBUG_GPIO_UARTPORT(CFG_FUNC_COMMUNICATION_TX_PORT), UART_IOCTL_DMA_RX_EN, 1);

	DMA_CircularConfig(CFG_FUNC_COMMUNICATION_RX_DMA_PORT, sizeof(uart_tx_buf)/2, uart_rx_buf, sizeof(uart_tx_buf));
	DMA_CircularConfig(CFG_FUNC_COMMUNICATION_TX_DMA_PORT, sizeof(uart_tx_buf)/2, uart_tx_buf, sizeof(uart_tx_buf));
	DMA_ChannelEnable(CFG_FUNC_COMMUNICATION_RX_DMA_PORT);
	DMA_ChannelEnable(CFG_FUNC_COMMUNICATION_TX_DMA_PORT);

	mv_mopen(&uart_recv_handle, robo_uart_recv_buf, ROBO_UART_BUF_MAX_LEN - 1, NULL);
}

void uart_data_entry(void)
{
	uint32_t len;
	uint8_t rxdata;

	if(DMA_CircularDataLenGet(CFG_FUNC_COMMUNICATION_RX_DMA_PORT) > 0)
	{
		memset(uart_buf, 0x00, sizeof(uart_buf));
		DMA_CircularDataGet(CFG_FUNC_COMMUNICATION_RX_DMA_PORT, uart_buf, 1);
		rxdata = uart_buf[0];
		mv_mwrite(&rxdata, 1, 1, &uart_recv_handle);
	}

	len = mv_msize(&uart_recv_handle);
	if(len > 0)
	{
//		DBG("read: %d\n", len);
		mv_mread(s_rx_buf, 1, len, &uart_recv_handle);
		roboeffect_prot_parse_big_block(s_rx_buf, len);
	}
}
#endif

void Communication_Effect_Config(uint8_t Control, uint8_t *buf, uint32_t len)
{
	// printf("see:%02X\n", Control);
	switch(Control)
	{
		case 0x00:
			Communication_Effect_0x00();
			break;
		case 0x01:
			Communication_Effect_0x01(buf, len);
			break;
		case 0x02:
			Communication_Effect_0x02();
			break;
		case 0x03:
			Communication_Effect_0x03(buf, len);
			break;
		case 0x04:
			Communication_Effect_0x04(buf, len);
			break;
		case 0x06:
			Communication_Effect_0x06(buf, len);
			break;
		case 0x07:
			Communication_Effect_0x07(buf, len);
			break;
		case 0x08:
			Communication_Effect_0x08(buf, len);
			break;
		case 0x09:
			Communication_Effect_0x09(buf, len);
			break;
		case 0x0A: //BP15 无DACX
//			Communication_Effect_0x0A(buf, len);
			break;
		case 0x0B://I2S0，不支持在线调音
			Communication_Effect_0x0B(buf, len);
			break;
		case 0x0C://I2S1，不支持在线调音
			Communication_Effect_0x0C(buf, len);
			break;
		case 0x0D://spdif，不支持在线调音
			Communication_Effect_0x0D(buf, len);
			break;
		case 0x80:
			Communication_Effect_0x80(buf, len);
			break;
		case 0xfb:
			 Communication_Effect_0xfb(buf, len);
			break;
		case 0xfc://user define tag
			 Communication_Effect_0xfc(buf, len);
			break;
		case 0xfd://user define tag
			 Communication_Effect_0xfd(buf, len);
			break;
		case 0xff:
			Communication_Effect_0xff(buf, len);
			break;
		default:
			if((Control >= 0x81) && (Control < 0xfb))
			{
				roboeffect_effect_update_params_entrance(Control, buf, len);
			}
			else
			{

			}
			break;
	}
	//-----Send ACK ----------------------//
	if(Control > 0xf0)///有效控制寄存器范围   这句解决加调音参数加密问题
	{
		return;
	}
	if(Control != 0x80 && package_id != 0)
	{
		package_id = 0;		//重新开始计数;
		gCtrlVars.AutoRefresh = AutoRefresh_ALL_PARA;
	}
	if((Control > 2)&&(Control != 0x80))
	{
		if(len > 0)// if(len = 0) {polling all parameter}
		{
			memset(tx_buf, 0, sizeof(tx_buf));
			tx_buf[0] = Control;
			Communication_Effect_Send(tx_buf, 1);
		}
	}
}
#endif
