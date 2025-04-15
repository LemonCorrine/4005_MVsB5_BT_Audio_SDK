#ifndef _USER_EFFECT_PARAMETER_H_
#define _USER_EFFECT_PARAMETER_H_
#include "audio_core_api.h"
#include "ctrlvars.h"
#include "eq.h"
#include "reverb.h"
#include "reverb_plate.h"
#include "reverb_pro.h"
#include "hfp\user_effect_flow_hfp.h"
#include "music\user_effect_flow_music.h"
#include "mic\user_effect_flow_mic.h"
#include "karaoke\user_effect_flow_karaoke.h"

typedef enum _ROBOEFFECT_EFFECT_MODE
{
    ROBOEFFECT_EFFECT_MODE_MIC = 0,
	ROBOEFFECT_EFFECT_MODE_MUSIC,
	ROBOEFFECT_EFFECT_MODE_HFP,
	ROBOEFFECT_EFFECT_MODE_HUNXIANG,
	ROBOEFFECT_EFFECT_MODE_DIANYIN,
	ROBOEFFECT_EFFECT_MODE_MOYIN,
	ROBOEFFECT_EFFECT_MODE_HANMAI,
	ROBOEFFECT_EFFECT_MODE_NANBIANNV,
	ROBOEFFECT_EFFECT_MODE_NVBIANNAN,
	ROBOEFFECT_EFFECT_MODE_WAWAYIN,
} ROBOEFFECT_EFFECT_MODE;

typedef struct _ROBOEFFECT_EFFECT_ADDR
{
	uint8_t MUSIC_EQ_ADDR;
	uint8_t MIC_EQ_ADDR;
	uint8_t REVERB_ADDR;
	uint8_t ECHO_ADDR;
	uint8_t SILENCE_DETECTOR_ADDR;
	uint8_t APP_SOURCE_GAIN_ADDR;
	uint8_t REMIND_SOURCE_GAIN_ADDR;
	uint8_t MIC_SOURCE_GAIN_ADDR;
	uint8_t REC_SOURCE_GAIN_ADDR;
	uint8_t DAC0_SINK_GAIN_ADDR;
	uint8_t APP_SINK_GAIN_ADDR;
	uint8_t STEREO_SINK_GAIN_ADDR;
	uint8_t REC_SINK_GAIN_ADDR;
} ROBOEFFECT_EFFECT_ADDR;

typedef enum _ROBOEFFECT_EFFECT_TYPE
{
	MUSIC_EQ = 0,
	MIC_EQ,
	REVERB,
	ECHO,
	SILENCE_DETECTOR,
	APP_SOURCE_GAIN,
	REMIND_SOURCE_GAIN,
	MIC_SOURCE_GAIN,
	REC_SOURCE_GAIN,
	DAC0_SINK_GAIN,
	APP_SINK_GAIN,
	STEREO_SINK_GAIN,
	REC_SINK_GAIN,
} ROBOEFFECT_EFFECT_TYPE;

typedef struct _ROBOEFFECT_SOURCE_NUM
{
	uint8_t mic_source;		//MIC_SOURCE_NUM	 //��˷�ͨ·
	uint8_t app_source;		//APP_SOURCE_NUM 	//app��Ҫ��Դͨ��
	uint8_t remind_source;	//PLAYBACK_SOURCE_NUM	//��ʾ��ʹ�ù̶�����ͨ��
	uint8_t rec_source;	//REMIND_SOURCE_NUM	//¼���ط�ͨ��
} ROBOEFFECT_SOURCE_NUM;

typedef struct _ROBOEFFECT_SINK_NUM
{
	uint8_t dac0_sink;		//AUDIO_DAC0_SINK_NUM		//����Ƶ�����audiocore Sink�е�ͨ�����������ã�audiocore���ô�ͨ��buf��������
	uint8_t app_sink;		//AUDIO_APP_SINK_NUM
	uint8_t stereo_sink;	//AUDIO_STEREO_SINK_NUM     //ģʽ�޹�Dac0֮��� ���������
	uint8_t rec_sink;	//AUDIO_RECORDER_SINK_NUM		//¼��ͨ��
} ROBOEFFECT_SINK_NUM;

typedef struct _ROBOEFFECT_EFFECT_PARA
{
	roboeffect_effect_list_info *user_effect_list;
	roboeffect_effect_steps_table *user_effect_steps;
	uint8_t *user_effects_script;

	uint8_t *user_effect_parameters;
	uint8_t *user_module_parameters;
	uint32_t (*get_user_effects_script_len)(void);
}ROBOEFFECT_EFFECT_PARA;

typedef struct __FilterParams
{
	uint16_t 	enable;
	int16_t		type;           /**< filter type, @see EQ_FILTER_TYPE_SET                               */
	uint16_t	f0;             /**< center frequency (peak) or mid-point frequency (shelf)             */
	int16_t		Q;              /**< quality factor (peak) or slope (shelf), format: Q6.10              */
	int16_t		gain;           /**< Gain in dB, format: Q8.8 */
} FilterParams;

typedef struct __EQUnit
{
	int16_t 		 pregain;
	uint16_t         calculation_type;
	FilterParams	 eq_params[10];
} EQUnit;

typedef struct __ReverbUnit
{
	int16_t 		 dry_scale;
	int16_t 		 wet_scale;
	int16_t 		 width_scale;
	int16_t 		 roomsize_scale;
	int16_t 		 damping_scale;
	uint16_t         mono;
} ReverbUnit;

typedef struct __ReverbPlateUnit
{
	int16_t highcut_freq;
	int16_t modulation_en;
	int16_t predelay;
	int16_t diffusion;
	int16_t decay;
	int16_t damping;
	int16_t wetdrymix;
} ReverbPlateUnit;

typedef struct __ReverbProUnit
{
	int16_t dry;
	int16_t wet;
	int16_t erwet;
	int16_t erfactor;
	int16_t erwidth;
	int16_t ertolate;
	int16_t rt60;
	int16_t delay;
	int16_t width;
	int16_t wander;
	int16_t spin;
	int16_t inputlpf;
	int16_t damplpf;
	int16_t basslpf;
	int16_t bassb;
	int16_t outputlpf;
} ReverbProUnit;

typedef struct __EchoUnit
{
	int16_t  		 fc;
	int16_t  		 attenuation;
	int16_t  		 delay;
	int16_t  		 max_delay;
	int16_t          quality_mode;//0,1=normal echo, 0,2=24bit echo
	int16_t          dry;
	int16_t          wet;
} EchoUnit;

typedef struct __SilenceDetectorUnit
{
	uint16_t        	   level;
} SilenceDetectorUnit;

typedef struct __GainControlUnit
{
	uint16_t      mute;
	uint16_t      gain;
} GainControlUnit;

typedef struct __ReverbMaxUnit
{
	int16_t  		 max_echo_delay;
	int16_t  		 max_echo_depth;
	int16_t  		 max_reverb_wet_scale;
	int16_t  		 max_reverb_roomsize;
} ReverbMaxUnit;

void Roboeffect_GetAudioEffectMaxValue(void);

void Roboeffect_EQ_Ajust(ROBOEFFECT_EFFECT_TYPE type,uint8_t BassGain, uint8_t TrebGain);

void Roboeffect_ReverbStep_Ajust(uint8_t ReverbStep);

uint16_t Roboeffect_SilenceDetector_Get(uint8_t node);

uint16_t Roboeffect_GainControl_Get(uint8_t node);

void Roboeffect_GainControl_Set(uint8_t node, uint16_t gain);

void Roboeffect_EQMode_Set(uint8_t EQMode);

void Roboeffect_SourceGain_Update(uint8_t Index);

void Roboeffect_SinkGain_Update(uint8_t Index);

void Roboeffect_SinkMute_Set(bool muteFlag);

void roboeffect_update_local_params(uint8_t addr, uint8_t param_index, int16_t *param_input, uint8_t param_len);

void roboeffect_update_local_block_params(uint8_t addr);

uint8_t AudioCoreSourceToRoboeffect(int8_t source);

uint8_t AudioCoreSinkToRoboeffect(int8_t sink);

uint16_t get_user_effect_parameters_len(uint8_t *user_effect_parameters);

ROBOEFFECT_EFFECT_PARA * get_user_effect_parameters(ROBOEFFECT_EFFECT_MODE mode);

roboeffect_effect_list_info *get_local_effect_list_buf(void);

uint8_t get_roboeffect_addr(ROBOEFFECT_EFFECT_TYPE effect_name);

uint16_t get_roboeffectVolArr(uint8_t vol);

#endif
