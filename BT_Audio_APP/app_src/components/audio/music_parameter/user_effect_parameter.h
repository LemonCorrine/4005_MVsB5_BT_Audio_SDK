#ifndef _USER_EFFECT_PARAMETER_H_
#define _USER_EFFECT_PARAMETER_H_
#include "audio_core_api.h"
#include "ctrlvars.h"
#include "eq.h"
#include "reverb.h"
#include "reverb_plate.h"
#include "reverb_pro.h"

#define AUDIOCORE_SOURCE_SINK_ERROR 0xFF

typedef struct _AUDIOEFFECT_EFFECT_ADDR
{
	uint8_t MUSIC_EQ_ADDR;
	uint8_t MIC_EQ_ADDR;
	uint8_t REVERB_ADDR;
	uint8_t REVERBPLATE_ADDR;
	uint8_t ECHO_ADDR;
	uint8_t VOCAL_CUT_ADDR;
	uint8_t SILENCE_DETECTOR_ADDR;
	uint8_t VOICE_CHANGER_ADDR;
	uint8_t APP_SOURCE_GAIN_ADDR;
	uint8_t REMIND_SOURCE_GAIN_ADDR;
	uint8_t MIC_SOURCE_GAIN_ADDR;
	uint8_t REC_SOURCE_GAIN_ADDR;
	uint8_t DAC0_SINK_GAIN_ADDR;
	uint8_t APP_SINK_GAIN_ADDR;
	uint8_t STEREO_SINK_GAIN_ADDR;
	uint8_t REC_SINK_GAIN_ADDR;
} AUDIOEFFECT_EFFECT_ADDR;

typedef enum _AUDIOEFFECT_EFFECT_TYPE
{
	MUSIC_EQ = 0,
	MIC_EQ,
	REVERB,
	REVERBPLATE,
	ECHO,
	VOCAL_CUT,
	SILENCE_DETECTOR,
	VOICE_CHANGER,
	APP_SOURCE_GAIN,
	I2S_MIX_SOURCE_GAIN,
	USB_SOURCE_GAIN,
	LINEIN_MIX_SOURCE_GAIN,
	REMIND_SOURCE_GAIN,
	MIC_SOURCE_GAIN,
	REC_SOURCE_GAIN,
	DAC0_SINK_GAIN,
	APP_SINK_GAIN,
	STEREO_SINK_GAIN,
	I2S_MIX_SINK_GAIN,
	REC_SINK_GAIN,
} AUDIOEFFECT_EFFECT_TYPE;

typedef struct _AUDIOEFFECT_SOURCE_NUM
{
	uint8_t mic_source;		//MIC_SOURCE_NUM	 //麦克风通路
	uint8_t app_source;		//APP_SOURCE_NUM 	//app主要音源通道
	uint8_t remind_source;	//PLAYBACK_SOURCE_NUM	//提示音使用固定混音通道
	uint8_t rec_source;		//REMIND_SOURCE_NUM	//录音回放通道
	uint8_t usb_source;     //USB_SOURCE        //USB MIX通道
	uint8_t i2s_mix_source; //I2S_MIX_SOURCE     //I2S MIX通道
	uint8_t linein_mix_source; //LINEIN_MIX_SOURCE_NUM     //LINE IN MIX通道
} AUDIOEFFECT_SOURCE_NUM;

typedef struct _AUDIOEFFECT_SINK_NUM
{
	uint8_t dac0_sink;		//AUDIO_DAC0_SINK_NUM		//主音频输出在audiocore Sink中的通道，必须配置，audiocore借用此通道buf处理数据
	uint8_t app_sink;		//AUDIO_APP_SINK_NUM
	uint8_t stereo_sink;	//AUDIO_STEREO_SINK_NUM     //模式无关Dac0之外的 立体声输出
	uint8_t rec_sink;		//AUDIO_RECORDER_SINK_NUM		//录音通道
	uint8_t i2s_mix_sink;  	//AUDIO_I2S_MIX_OUT_SINK_NUM //I2S MIX OUT通道
	uint8_t spdif_sink;		//AUDIO_SPDIF_SINK_NUM
} AUDIOEFFECT_SINK_NUM;

typedef struct _AUDIOEFFECT_EFFECT_PARA
{
	roboeffect_effect_list_info *user_effect_list;
	roboeffect_effect_steps_table *user_effect_steps;
	uint8_t *user_effects_script;
	uint8_t *user_effect_name;

	uint8_t *user_effect_parameters;
	uint8_t *user_module_parameters;
	uint32_t (*get_user_effects_script_len)(void);
}AUDIOEFFECT_EFFECT_PARA;

typedef struct _AUDIOEFFECT_EFFECT_PARA_TABLE
{
	uint32_t		effect_id;
	uint32_t		effect_id_count;
	AUDIOEFFECT_EFFECT_ADDR effect_addr;
	AUDIOEFFECT_SOURCE_NUM audioeffect_source;
	AUDIOEFFECT_SINK_NUM audioeffect_sink;
	AUDIOEFFECT_EFFECT_PARA *audioeffect_para;
}AUDIOEFFECT_EFFECT_PARA_TABLE;

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
	int16_t  		 max_reverbplate_wetdrymix;
} ReverbMaxUnit;

void AudioEffect_GetAudioEffectMaxValue(void);

void AudioEffect_EQ_Ajust(AUDIOEFFECT_EFFECT_TYPE type, uint8_t BassGain, uint8_t TrebGain);

void AudioEffect_ReverbStep_Ajust(uint8_t ReverbStep);

void AudioEffect_ReverbPlateStep_Ajust(uint8_t ReverbStep);

uint16_t AudioEffect_SilenceDetector_Get(AUDIOEFFECT_EFFECT_TYPE type);

uint16_t AudioEffect_GainControl_Get(AUDIOEFFECT_EFFECT_TYPE type);

void AudioEffect_GainControl_Set(AUDIOEFFECT_EFFECT_TYPE type, uint16_t gain);

void AudioEffect_EQMode_Set(uint8_t EQMode);

void AudioEffect_SourceGain_Update(uint8_t Index);

void AudioEffect_SinkGain_Update(uint8_t Index);

void AudioEffect_update_local_params(uint8_t addr, uint8_t param_index, int16_t *param_input, uint8_t param_len);

void AudioEffect_update_local_effect_status(uint8_t addr, uint8_t effect_enable);

void AudioEffect_update_local_block_params(uint8_t addr);

uint8_t AudioEffect_effect_status_Get(AUDIOEFFECT_EFFECT_TYPE type);

void AudioEffect_effect_enable(AUDIOEFFECT_EFFECT_TYPE type, uint8_t enable);

uint8_t AudioCoreSourceToRoboeffect(int8_t source);

uint8_t AudioCoreSinkToRoboeffect(int8_t sink);

uint16_t get_user_effect_parameters_len(uint8_t *user_effect_parameters);

AUDIOEFFECT_EFFECT_PARA * get_user_effect_parameters(uint8_t mode);

uint8_t get_audioeffect_addr(AUDIOEFFECT_EFFECT_TYPE effect_name);

uint16_t get_audioeffectVolArr(uint8_t vol);

void AudioEffectParamSync(void);

bool AudioEffect_effectAddr_check(uint8_t node);
#endif
