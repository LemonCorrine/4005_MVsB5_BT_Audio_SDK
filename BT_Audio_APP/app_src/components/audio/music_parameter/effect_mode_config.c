#include <nds32_utils_math.h>
#include <string.h>
#include <math.h>
#include "roboeffect_api.h"
#include "user_defined_effect_api.h"
#include "user_effect_parameter.h"
#include "nn_denoise_api.h"
#include "main_task.h"
#include "bt_config.h"
#include "breakpoint.h"
#include "auto_gen_msg_process.h"
#include "communication.h"

typedef enum
{
	EffectModeStateSuspend	= 0,//EffectMode����̬����������ֱ���л�������Чģʽ���̶�ģʽʹ�ã�����HFP AEC
	EffectModeStateReady,		//EffectMode����̬�����������л�
}EffectModeState;

typedef struct __UserEffectModeValidConfig
{
	uint8_t 						effect_mode;
	EffectModeState 				effect_mode_state;
	const AUDIOEFFECT_SOURCE_SINK_NUM *source_sink;
	const AUDIOEFFECT_EFFECT_PARA	 *effect_para;
	const uint8_t 					*effect_control;
	uint8_t 						(*msg_process)(int msg);
}UserEffectModeValidConfig;

extern const AUDIOEFFECT_EFFECT_PARA bypass_effect_para;
extern const AUDIOEFFECT_SOURCE_SINK_NUM bypass_mode;

extern const AUDIOEFFECT_EFFECT_PARA HunXiang_effect_para;
extern const AUDIOEFFECT_EFFECT_PARA DianYin_effect_para;
extern const AUDIOEFFECT_EFFECT_PARA MoYin_effect_para;
extern const AUDIOEFFECT_EFFECT_PARA HanMai_effect_para;
extern const AUDIOEFFECT_EFFECT_PARA NanBianNv_effect_para;
extern const AUDIOEFFECT_EFFECT_PARA NvBianNan_effect_para;
extern const AUDIOEFFECT_EFFECT_PARA WaWaYin_effect_para;
extern const AUDIOEFFECT_SOURCE_SINK_NUM Karaoke_mode;
extern const uint8_t Karaoke_effect_ctrl[AUDIOEFFECT_EFFECT_CONTROL_MAX];

extern const AUDIOEFFECT_EFFECT_PARA mic_effect_para;
extern const AUDIOEFFECT_SOURCE_SINK_NUM mic_mode;
extern const uint8_t mic_effect_ctrl[AUDIOEFFECT_EFFECT_CONTROL_MAX];

extern const AUDIOEFFECT_EFFECT_PARA music_effect_para;
extern const AUDIOEFFECT_SOURCE_SINK_NUM music_mode;
extern const uint8_t music_effect_ctrl[AUDIOEFFECT_EFFECT_CONTROL_MAX];

extern const AUDIOEFFECT_EFFECT_PARA micusbAI_effect_para;
extern const AUDIOEFFECT_SOURCE_SINK_NUM micusbAI_mode;

extern const AUDIOEFFECT_EFFECT_PARA hfp_effect_para;
extern const AUDIOEFFECT_SOURCE_SINK_NUM hfp_mode;
extern const uint8_t hfp_effect_ctrl[AUDIOEFFECT_EFFECT_CONTROL_MAX];

extern const AUDIOEFFECT_EFFECT_PARA uac_effect_para;
extern const AUDIOEFFECT_SOURCE_SINK_NUM uac_mode;
extern const uint8_t uac_effect_ctrl[AUDIOEFFECT_EFFECT_CONTROL_MAX];

extern const AUDIOEFFECT_EFFECT_PARA TwoAudio_effect_para;
extern const AUDIOEFFECT_SOURCE_SINK_NUM TwoAudio_mode;
extern const uint8_t TwoAudio_effect_ctrl[AUDIOEFFECT_EFFECT_CONTROL_MAX];

#ifndef CFG_FLOWCHART_KARAOKE_ENABLE
uint8_t msg_process_Karaoke(int msg)
{
	return 0;
}
#endif

//��Чģʽ�����л����ñ�˳���л�
//������Ч�����ڵ㣬MSG��Ϣ����
static const UserEffectModeValidConfig EffectModeToggleMap[] =
{
#if(CFG_FUNC_EFFECT_DEMO == EFFECT_USB_HOST_AUDIO_DEMO) 		//host uac ��Ч��ͼDEMO
	{EFFECT_MODE_UAC,		EffectModeStateReady,	&uac_mode,		&uac_effect_para,		uac_effect_ctrl,	NULL	},
#elif(CFG_FUNC_EFFECT_DEMO == EFFECT_USB_DEVICE_TWO_AUDIO_DEMO)
	{EFFECT_MODE_TWO_AUDIO,	EffectModeStateReady,	&TwoAudio_mode,	&TwoAudio_effect_para,	TwoAudio_effect_ctrl,	NULL	},
#else
#ifdef CFG_AI_DENOISE_EN
	{EFFECT_MODE_MICUSBAI,	EffectModeStateReady,	&micusbAI_mode,	&micusbAI_effect_para,	NULL,	NULL},
#endif
#ifdef CFG_FUNC_EFFECT_BYPASS_EN
	{EFFECT_MODE_BYPASS,	EffectModeStateReady,	&bypass_mode,	&bypass_effect_para,	NULL,	NULL},
#else
	#ifdef CFG_FUNC_MIC_KARAOKE_EN
		{EFFECT_MODE_HunXiang,	EffectModeStateReady,	&Karaoke_mode,	&HunXiang_effect_para,	Karaoke_effect_ctrl,	msg_process_Karaoke},
		{EFFECT_MODE_DianYin,	EffectModeStateReady,	&Karaoke_mode,	&DianYin_effect_para,	Karaoke_effect_ctrl,	msg_process_Karaoke},
		{EFFECT_MODE_MoYin,		EffectModeStateReady,	&Karaoke_mode,	&MoYin_effect_para,		Karaoke_effect_ctrl,	msg_process_Karaoke},
		{EFFECT_MODE_HanMai,	EffectModeStateReady,	&Karaoke_mode,	&HanMai_effect_para,	Karaoke_effect_ctrl,	msg_process_Karaoke},
		{EFFECT_MODE_NanBianNv,	EffectModeStateReady,	&Karaoke_mode,	&NanBianNv_effect_para,	Karaoke_effect_ctrl,	msg_process_Karaoke},
		{EFFECT_MODE_NvBianNan,	EffectModeStateReady,	&Karaoke_mode,	&NvBianNan_effect_para,	Karaoke_effect_ctrl,	msg_process_Karaoke},
		{EFFECT_MODE_WaWaYin,	EffectModeStateReady,	&Karaoke_mode,	&WaWaYin_effect_para,	Karaoke_effect_ctrl,	msg_process_Karaoke},
	#else
		{EFFECT_MODE_MIC,		EffectModeStateReady,	&mic_mode,		&mic_effect_para,		mic_effect_ctrl,	NULL	},
		{EFFECT_MODE_MUSIC,		EffectModeStateReady,	&music_mode,	&music_effect_para,		music_effect_ctrl,	NULL	},
	#endif
#endif
#if defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT)
	{EFFECT_MODE_HFP_AEC,	EffectModeStateSuspend,	&hfp_mode,		&hfp_effect_para,	hfp_effect_ctrl,	NULL},
#endif
#endif
};

#define 	EffectModeToggleMapMaxIndex		(sizeof(EffectModeToggleMap)/sizeof(EffectModeToggleMap[0]))

//��ȡ��ͼ��source/sink����
AUDIOEFFECT_SOURCE_SINK_NUM * get_user_effect_source_sink(void)
{
	uint32_t i = 0;

	for(i=0;i<EffectModeToggleMapMaxIndex;i++)
	{
		if(mainAppCt.EffectMode == EffectModeToggleMap[i].effect_mode)
			return (AUDIOEFFECT_SOURCE_SINK_NUM *)EffectModeToggleMap[i].source_sink;
	}
	return (AUDIOEFFECT_SOURCE_SINK_NUM *)EffectModeToggleMap[0].source_sink;
}

//��ȡ��ͼ��Ч����
AUDIOEFFECT_EFFECT_PARA * get_user_effect_parameters(uint8_t mode)
{
	uint32_t i = 0;

	for(i=0;i<EffectModeToggleMapMaxIndex;i++)
	{
		if(mode == EffectModeToggleMap[i].effect_mode)
			return (AUDIOEFFECT_EFFECT_PARA *)EffectModeToggleMap[i].effect_para;
	}
	return (AUDIOEFFECT_EFFECT_PARA *)EffectModeToggleMap[0].effect_para;
}

//��ȡĬ�ϵĵ�һ����Ч
uint8_t GetFristEffectMode(void)
{
	uint8_t i;

	for(i = 0; i < EffectModeToggleMapMaxIndex; i++)
	{
		if(EffectModeToggleMap[i].effect_mode_state != EffectModeStateSuspend)
		{
			return EffectModeToggleMap[i].effect_mode;
		}
	}

	return EffectModeToggleMap[0].effect_mode;
}

//�л�����һ����Чģʽ
uint8_t GetNextEffectMode(void)
{
	uint8_t i,j;

	for(i = 0; i < EffectModeToggleMapMaxIndex; i++)
	{
		if(mainAppCt.EffectMode == EffectModeToggleMap[i].effect_mode)
		{	//�л�����һ����Чģʽ
			for(j = (i+1) % EffectModeToggleMapMaxIndex; j != i ; j = (j+1) % EffectModeToggleMapMaxIndex)
			{
				if(EffectModeToggleMap[j].effect_mode_state != EffectModeStateSuspend)
				{
					return EffectModeToggleMap[j].effect_mode;
				}
			}
		}
	}

	//û���ҵ��Ϸ�����Чģʽ�����л�
	return mainAppCt.EffectMode;

	//û���ҵ���Чģʽ��Ĭ�ϵ�һ��
//	return GetFristEffectMode();
}

//��Ч������Ϣ����
uint8_t EffectModeMsgProcess(uint16_t Msg)
{
	uint8_t i;

	for(i = 0; i < EffectModeToggleMapMaxIndex; i++)
	{
		if(mainAppCt.EffectMode == EffectModeToggleMap[i].effect_mode &&
		   EffectModeToggleMap[i].msg_process)
		{
			return EffectModeToggleMap[i].msg_process(Msg);
		}
	}

	return 0;
}

//�Ƚ�2��Mode�Ƿ���ͬһ����ͼ
bool EffectModeCmp(uint8_t mode1,uint8_t mode2)
{
	uint8_t i,index1,index2;

	index1 = 0xff;
	index2 = 0xff;
	for(i = 0; i < EffectModeToggleMapMaxIndex; i++)
	{
		if(mode1 == EffectModeToggleMap[i].effect_mode)
			index1 = i;
		if(mode2 == EffectModeToggleMap[i].effect_mode)
			index2 = i;
		if(index1 != 0xff && index2 != 0xff)
		{
			if(EffectModeToggleMap[index1].source_sink == EffectModeToggleMap[index2].source_sink)
				return 1;
			else
				return 0;
		}
	}

	return 0;
}

//��ȡ��ǰ��ͼ�ж��ٸ���Ч����
uint8_t GetEffectModeParaCount(void)
{
	uint8_t i,count,index = 0xff;

	count = 0;

	for(i=0;i<EffectModeToggleMapMaxIndex;i++)
	{
		if(mainAppCt.EffectMode == EffectModeToggleMap[i].effect_mode)
		{
			index = i;
			break;
		}
	}

	if(index < EffectModeToggleMapMaxIndex)
	{
		for(i = 0; i < EffectModeToggleMapMaxIndex; i++)
		{
			if(EffectModeToggleMap[i].source_sink == EffectModeToggleMap[index].source_sink)
				count++;
		}
	}

	return count;
}

//��ȡ��num����Ч���� �ڵ�ǰ��ͼ�е��������
uint8_t	GetEffectModeParaIndex(uint8_t num)
{
	uint8_t i,cur_index = 0xff;

	for(i=0;i<EffectModeToggleMapMaxIndex;i++)
	{
		if(mainAppCt.EffectMode == EffectModeToggleMap[i].effect_mode)
		{
			cur_index = i; 	// ��¼��ǰ��Ч�±�
			break;
		}
	}

	if(cur_index < EffectModeToggleMapMaxIndex)
	{
		for(i = 0; i < EffectModeToggleMapMaxIndex; i++)
		{
			if(EffectModeToggleMap[i].source_sink == EffectModeToggleMap[cur_index].source_sink)
			{
				if(num == 0)
					return i; //�ҵ������ز���
				else
					num--;
			}
		}

		return cur_index; //���󣬷���ԭ������
	}

	return 0;
}

//�ҵ���ǰ��ͼ�еĵ�index����Ч��effect mode
uint8_t GetCurSameEffectModeID(uint8_t index)
{
	return EffectModeToggleMap[GetEffectModeParaIndex(index)].effect_mode;
}

//�ҵ���ǰ��ͼ�еĵ�index����Ч�Ĳ���
AUDIOEFFECT_EFFECT_PARA * GetCurSameEffectModeAudioPara(uint8_t index)
{
	return (AUDIOEFFECT_EFFECT_PARA *)EffectModeToggleMap[GetEffectModeParaIndex(index)].effect_para;
}


//��ȡeffect mode �ڵ�ǰ��ͼ��Ч�����ı��
uint8_t GetCurEffectModeNum(void)
{
	uint8_t i,num,index = 0xff;

	num = 0;
	for(i=0;i<EffectModeToggleMapMaxIndex;i++)
	{
		if(mainAppCt.EffectMode == EffectModeToggleMap[i].effect_mode)
		{
			index = i;
			break;
		}
	}

	if(index < EffectModeToggleMapMaxIndex)
	{
		for(i = 0; i < index; i++)
		{
			if(EffectModeToggleMap[i].source_sink == EffectModeToggleMap[index].source_sink)
				num++;
		}
	}

	return num;
}

//��ȡ��Ч���ƽڵ�index
uint8_t GetEffectControlIndex(AUDIOEFFECT_EFFECT_CONTROL type)
{
	uint8_t i;

	if(type >= AUDIOEFFECT_EFFECT_CONTROL_MAX)
		return 0;

	for(i = 0; i < EffectModeToggleMapMaxIndex; i++)
	{
		if(mainAppCt.EffectMode == EffectModeToggleMap[i].effect_mode &&
		   EffectModeToggleMap[i].effect_control)
		{
			return EffectModeToggleMap[i].effect_control[type];
		}
	}

	return 0;
}

uint32_t GetEffectNameTotalLen()
{
	uint8_t  index = 0;
	uint32_t len   = 0;
	AUDIOEFFECT_EFFECT_PARA *para;
	for (index = 0; index < GetEffectModeParaCount(); index++)
	{
		para = GetCurSameEffectModeAudioPara(index);
		len += strlen((char *)para->user_effect_name);
	}
	return len;
}

void GetEffectNameCurPackageforCommunication(uint32_t done_len, uint8_t *p)
{
    uint8_t index, name_len = 0;
    uint32_t data_len = 0;
    AUDIOEFFECT_EFFECT_PARA *para;

    for (index = 0; index < GetEffectModeParaCount(); index++)
    {
        para = GetCurSameEffectModeAudioPara(index);
        uint32_t name_length = strlen((char *)para->user_effect_name);

        if (data_len + name_length >= done_len)
        {
            name_len = (data_len + name_length) - done_len;
            if (name_len == 0)
            {
                name_len = name_length;
                if (done_len > 0)
                {
                    *p = ';';
                    data_len = 1;
                    index++;     // current effect name have already in last package, skip it
                }
            }
            else
            {
            	data_len = 0;
            }
            break;
        }
        data_len += (name_length + 1); // +1 for ';'
    }

    for (; index < GetEffectModeParaCount(); index++)
    {
        para = GetCurSameEffectModeAudioPara(index);
        uint32_t total_name_len = strlen((char *)para->user_effect_name);
        name_len = name_len ? name_len : total_name_len;

        uint32_t available = STREAM_CLIPS_LEN - data_len;

        if (index == (GetEffectModeParaCount() - 1))
        {
            uint32_t copy_len = (available >= name_len) ? name_len : available;
            memcpy(p + data_len, para->user_effect_name + (total_name_len - name_len), copy_len);
            data_len += copy_len;
            if (copy_len < name_len)
            	return;
        }
        else
        {
            uint32_t required = name_len + 1; // name + ';'

            if (available >= required)
            {
                memcpy(p + data_len, para->user_effect_name + (total_name_len - name_len), name_len);
                data_len += name_len;
                p[data_len++] = ';';
            }
            else if	(available > 0)
            {
                memcpy(p + data_len, para->user_effect_name + (total_name_len - name_len), available);
                data_len += available;
                return;
            }
        }
        name_len = 0; // Reset for next iteration
    }
}
