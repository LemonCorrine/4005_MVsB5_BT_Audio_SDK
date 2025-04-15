/**
 **************************************************************************************
 * @file    user_defined_effect_api.h
 * @brief   interface for user defined effect algorithm
 *
 * @author  Castle Cai
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include "roboeffect_api.h"
#include "roboeffect_config.h"
#include "audio_core_api.h"
#include "ctrlvars.h"
#include "eq.h"
#include "reverb.h"
#include "reverb_plate.h"
#include "reverb_pro.h"
#include "user_effect_flow_hfp.h"
#include "user_effect_flow_music.h"
#include "user_effect_flow_mic.h"
#include "user_effect_flow_karaoke.h"

#define AI_MEMORY_SIZE 70976
#define AI_SCRATCH_SIZE 10296
#define AI_RAM_SIZE 54988

extern roboeffect_effect_list_info local_effect_list;

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
ROBOEFFECT_EFFECT_MODE effect_mode;

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
	uint8_t DAC0_SINK_GAIN_ADDR;
	uint8_t APP_SINK_GAIN_ADDR;
	uint8_t STEREO_SINK_GAIN_ADDR;
} ROBOEFFECT_EFFECT_ADDR;
extern const ROBOEFFECT_EFFECT_ADDR effect_addr[];

typedef struct _ROBOEFFECT_SOURCE_NUM
{
	uint8_t mic_source;		//MIC_SOURCE_NUM	 //麦克风通路
	uint8_t app_source;		//APP_SOURCE_NUM 	//app主要音源通道
	uint8_t remind_source;	//REMIND_SOURCE_NUM	//提示音使用固定混音通道
} ROBOEFFECT_SOURCE_NUM;
extern const ROBOEFFECT_SOURCE_NUM roboeffect_source[];

typedef struct _ROBOEFFECT_SINK_NUM
{
	uint8_t dac0_sink;		//AUDIO_DAC0_SINK_NUM		//主音频输出在audiocore Sink中的通道，必须配置，audiocore借用此通道buf处理数据
	uint8_t app_sink;		//AUDIO_APP_SINK_NUM
	uint8_t stereo_sink;	//AUDIO_STEREO_SINK_NUM     //模式无关Dac0之外的 立体声输出
} ROBOEFFECT_SINK_NUM;
extern const ROBOEFFECT_SINK_NUM roboeffect_sink[];

typedef enum _roboeffect_user_effect_type_enum
{
	ROBOEFFECT_AI_DENOISE = ROBOEFFECT_USER_DEFINED_EFFECT_BEGIN,
	ROBOEFFECT_USER_GAIN,
} roboeffect_user_effect_type_enum;


typedef struct _ai_denoise_struct
{
	void *p_nn_denoise;
	float float_buffer_in[320];
	float float_buffer_out[320];
} ai_denoise_struct;


typedef struct _user_gain_struct
{
	int16_t data_a;
	int16_t data_b;
} user_gain_struct;


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

void Roboeffect_EQ_Ajust(uint8_t node, uint8_t BassGain, uint8_t TrebGain);

void Roboeffect_ReverbStep_Ajust(uint8_t Reverbnode, uint8_t Echonode, uint8_t ReverbStep);

uint16_t Roboeffect_SilenceDetector_Get(uint8_t node);

uint16_t Roboeffect_GainControl_Get(uint8_t node);

void Roboeffect_GainControl_Set(uint8_t node, uint16_t gain);

void Roboeffect_EQMode_Set(uint8_t EQMode);

void Roboeffect_SourceGain_Update(uint8_t Index);

void Roboeffect_SinkGain_Update(uint8_t Index);

void roboeffect_update_local_params(uint8_t addr, uint8_t param_index, int16_t *param_input, uint8_t param_len);

void roboeffect_update_local_block_params(uint8_t addr);

uint8_t AudioCoreSourceToRoboeffect(int8_t source);

uint8_t AudioCoreSinkToRoboeffect(int8_t sink);

uint16_t get_user_effect_parameters_len(uint8_t *user_effect_parameters);

extern uint32_t get_user_effects_script_len_mic(void);
extern uint32_t get_user_effects_script_len_music(void);
extern uint32_t get_user_effects_script_len_hfp(void);
extern uint32_t get_user_effects_script_len_Karaoke(void);

extern const unsigned char user_effect_parameters_hfp_hfp[];
extern const unsigned char user_module_parameters_hfp_hfp[];

extern const unsigned char user_effect_parameters_mic_mic[];
extern const unsigned char user_module_parameters_mic_mic[];

extern const unsigned char user_effect_parameters_music_music[];
extern const unsigned char user_module_parameters_music_music[];

extern const unsigned char user_effect_parameters_Karaoke_HunXiang[];
extern const unsigned char user_module_parameters_Karaoke_HunXiang[];
extern const unsigned char user_effect_parameters_Karaoke_DianYin[];
extern const unsigned char user_module_parameters_Karaoke_DianYin[];
extern const unsigned char user_effect_parameters_Karaoke_MoYin[];
extern const unsigned char user_module_parameters_Karaoke_MoYin[];
extern const unsigned char user_effect_parameters_Karaoke_HanMai[];
extern const unsigned char user_module_parameters_Karaoke_HanMai[];
extern const unsigned char user_effect_parameters_Karaoke_NanBianNv[];
extern const unsigned char user_module_parameters_Karaoke_NanBianNv[];
extern const unsigned char user_effect_parameters_Karaoke_NvBianNan[];
extern const unsigned char user_module_parameters_Karaoke_NvBianNan[];
extern const unsigned char user_effect_parameters_Karaoke_WaWaYin[];
extern const unsigned char user_module_parameters_Karaoke_WaWaYin[];
