/**
 *************************************************************************************
 * @file	user_effect_parameter.h
 * @brief	Audio effect control interface provided by SDK
 *
 * &copy; Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#ifndef _USER_EFFECT_PARAMETER_H_
#define _USER_EFFECT_PARAMETER_H_
#include "audio_core_api.h"
#include "ctrlvars.h"
#include "eq.h"
#include "reverb.h"
#include "reverb_plate.h"
#include "reverb_pro.h"

#define AUDIOCORE_SOURCE_SINK_ERROR 0xFF

/**
 * @brief Audio effect addr
 */
typedef struct _AUDIOEFFECT_EFFECT_ADDR
{
	uint8_t MUSIC_EQ_ADDR;
	uint8_t MIC_EQ_ADDR;
	uint8_t REVERB_ADDR;
	uint8_t REVERBPLATE_ADDR;
	uint8_t ECHO_ADDR;
	uint8_t VOCAL_CUT_ADDR;
	uint8_t SILENCE_DETECTOR_ADDR;
	uint8_t SILENCE_DETECTOR_MUSIC_ADDR;
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

/**
 * @brief Audio effect num(SDK defined)
 */
typedef enum _AUDIOEFFECT_EFFECT_NUM
{
	MUSIC_EQ = 0,
	MIC_EQ,
	REVERB,
	REVERBPLATE,
	ECHO,
	VOCAL_CUT,
	SILENCE_DETECTOR,
	SILENCE_DETECTOR_MUSIC,
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
} AUDIOEFFECT_EFFECT_NUM;

/**
 * @brief Audio effect source num
 */
typedef struct _AUDIOEFFECT_SOURCE_NUM
{
	uint8_t		mic_source;			/**< MIC_SOURCE_NUM                          */
	uint8_t		app_source;			/**< APP_SOURCE_NUM                          */
	uint8_t		remind_source;		/**< PLAYBACK_SOURCE_NUM                     */
	uint8_t		rec_source;			/**< REMIND_SOURCE_NUM                       */
	uint8_t		usb_source;			/**< USB_SOURCE                              */
	uint8_t		i2s_mix_source;		/**< I2S_MIX_SOURCE                          */
	uint8_t		linein_mix_source;	/**< LINEIN_MIX_SOURCE_NUM                   */
} AUDIOEFFECT_SOURCE_NUM;

/**
 * @brief Audio effect sink num
 */
typedef struct _AUDIOEFFECT_SINK_NUM
{
	uint8_t		dac0_sink;			/**< AUDIO_DAC0_SINK_NUM                       */
	uint8_t		app_sink;			/**< AUDIO_APP_SINK_NUM                        */
	uint8_t		stereo_sink;		/**< AUDIO_STEREO_SINK_NUM                     */
	uint8_t		rec_sink;			/**< AUDIO_RECORDER_SINK_NUM                   */
	uint8_t		i2s_mix_sink;		/**< AUDIO_I2S_MIX_OUT_SINK_NUM                */
	uint8_t		spdif_sink;			/**< AUDIO_SPDIF_SINK_NUM                      */
} AUDIOEFFECT_SINK_NUM;

/**
 * @brief Effect parameters for roboeffect engine
 */
typedef struct _AUDIOEFFECT_EFFECT_PARA
{
	roboeffect_effect_list_info 	*user_effect_list;
	roboeffect_effect_steps_table 	*user_effect_steps;
	uint8_t 						*user_effects_script;
	uint8_t 						*user_effect_name;

	uint8_t 						*user_effect_parameters;
	uint8_t 						*user_module_parameters;
	uint32_t 						(*get_user_effects_script_len)(void);
}AUDIOEFFECT_EFFECT_PARA;

/**
 * @brief Audio Effect parameters table
 */
typedef struct _AUDIOEFFECT_EFFECT_PARA_TABLE
{
	uint32_t				effect_id;
	uint32_t				effect_id_count;
	AUDIOEFFECT_EFFECT_ADDR effect_addr;
	AUDIOEFFECT_SOURCE_NUM 	audioeffect_source;
	AUDIOEFFECT_SINK_NUM 	audioeffect_sink;
	AUDIOEFFECT_EFFECT_PARA *audioeffect_para;
}AUDIOEFFECT_EFFECT_PARA_TABLE;

/**
 * @brief Filter parameters
 */
typedef struct __FilterParams
{
	uint16_t	enable;
	int16_t		type;           /**< filter type, @see EQ_FILTER_TYPE_SET                               */
	uint16_t	f0;             /**< center frequency (peak) or mid-point frequency (shelf)             */
	int16_t		Q;              /**< quality factor (peak) or slope (shelf), format: Q6.10              */
	int16_t		gain;           /**< Gain in dB, format: Q8.8 */
} FilterParams;

/**
 * @brief EQ unit
 */
typedef struct __EQUnit
{
	int16_t			pregain;
	uint16_t		calculation_type;
	FilterParams	eq_params[10];
} EQUnit;

/**
 * @brief Reverb unit
 */
typedef struct __ReverbUnit
{
	int16_t		dry_scale;
	int16_t		wet_scale;
	int16_t		width_scale;
	int16_t		roomsize_scale;
	int16_t		damping_scale;
	uint16_t	mono;
} ReverbUnit;

/**
 * @brief ReverbPlate unit
 */
typedef struct __ReverbPlateUnit
{
	int16_t		highcut_freq;
	int16_t		modulation_en;
	int16_t		predelay;
	int16_t		diffusion;
	int16_t		decay;
	int16_t		damping;
	int16_t		wetdrymix;
} ReverbPlateUnit;

/**
 * @brief ReverbPro unit
 */
typedef struct __ReverbProUnit
{
	int16_t		dry;
	int16_t		wet;
	int16_t		erwet;
	int16_t		erfactor;
	int16_t		erwidth;
	int16_t		ertolate;
	int16_t		rt60;
	int16_t		delay;
	int16_t		width;
	int16_t		wander;
	int16_t		spin;
	int16_t		inputlpf;
	int16_t		damplpf;
	int16_t		basslpf;
	int16_t		bassb;
	int16_t		outputlpf;
} ReverbProUnit;

/**
 * @brief Echo unit
 */
typedef struct __EchoUnit
{
	int16_t		fc;
	int16_t		attenuation;
	int16_t		delay;
	int16_t		max_delay;
	int16_t		quality_mode;//0,1=normal echo, 0,2=24bit echo
	int16_t		dry;
	int16_t		wet;
} EchoUnit;

/**
 * @brief SilenceDetector unit
 */
typedef struct __SilenceDetectorUnit
{
	uint16_t	level;
} SilenceDetectorUnit;

/**
 * @brief GainControl unit
 */
typedef struct __GainControlUnit
{
	uint16_t	mute;
	uint16_t	gain;
} GainControlUnit;

/**
 * @brief Reverb max parameters unit
 */
typedef struct __ReverbMaxUnit
{
	int16_t		max_echo_delay;
	int16_t		max_echo_depth;
	int16_t		max_reverb_wet_scale;
	int16_t		max_reverb_roomsize;
	int16_t		max_reverbplate_wetdrymix;
} ReverbMaxUnit;

/**
 * @brief  Get Reverb,Echo and ReverbPlate some default parameters as the max threshold
 * @param  void
 * @return None
 */
void AudioEffect_GetAudioEffectMaxValue(void);

/**
 * @brief  Set EQ bass gain and treb gain
 * @param  effect : effect EQ
 * @param  BassGain : bass gain level, 16 levels in BassTrebGainTable
 * @param  TrebGain : treb gain level, 16 levels in BassTrebGainTable
 * @return None
 */
void AudioEffect_EQ_Ajust(AUDIOEFFECT_EFFECT_NUM effect, uint8_t BassGain, uint8_t TrebGain);

/**
 * @brief  Set Echo and Reverb reverb step
 * @param  ReverbStep : reverb step
 * @return None
 */
void AudioEffect_ReverbStep_Ajust(uint8_t ReverbStep);

/**
 * @brief  Set Echo and ReverbPlate reverb step
 * @param  ReverbStep : reverb step
 * @return None
 */
void AudioEffect_ReverbPlateStep_Ajust(uint8_t ReverbStep);

/**
 * @brief  Get SilenceDetector level
 * @param  effect : expected audio effect
 * @return uint16_t : SilenceDetector level
 */
int32_t AudioEffect_SilenceDetector_Get(AUDIOEFFECT_EFFECT_NUM effect);

/**
 * @brief  Get GainControl gain level
 * @param  effect : expected audio effect
 * @return uint16_t : GainControl gain level
 */
uint16_t AudioEffect_GainControl_Get(AUDIOEFFECT_EFFECT_NUM effect);

/**
 * @brief  Set GainControl gain level
 * @param  effect : expected audio effect
 * @param  gain : GainControl gain level
 * @return None
 */
void AudioEffect_GainControl_Set(AUDIOEFFECT_EFFECT_NUM effect, uint16_t gain);

/**
 * @brief  Set MUSIC_EQ EQ mode
 * @param  EQMode : 6 modes in enum EQ_MODE
 * @return None
 */
void AudioEffect_EQMode_Set(uint8_t EQMode);

/**
 * @brief  Set source gain
 * @param  source : AudioCore source enum less than AUDIO_CORE_SOURCE_MAX_NUM
 * @return None
 */
void AudioEffect_SourceGain_Update(uint8_t source);

/**
 * @brief  Set sink gain
 * @param  sink : AudioCore sink enum less than AUDIO_CORE_SINK_MAX_NUM
 * @return None
 */
void AudioEffect_SinkGain_Update(uint8_t sink);

/**
 * @brief  Update an audio effect parameter of SDK local backup
 * @param  addr : expected audio effect addr
 * @param  param_index : parameter index in an audio effect parameters
 * @param  param_input : parameter
 * @param  param_len : parameter length
 * @return None
 */
void AudioEffect_update_local_params(uint8_t addr, uint8_t param_index, int16_t *param_input, uint8_t param_len);

/**
 * @brief  Update audio effect status of SDK local backup
 * @param  addr : expected audio effect addr
 * @param  effect_enable : 0 or 1
 * @return None
 */
void AudioEffect_update_local_effect_status(uint8_t addr, uint8_t effect_enable);

/**
 * @brief  Update a block effect parameters of SDK local backup
 * @param  addr : expected audio effect addr
 * @return None
 */
void AudioEffect_update_local_block_params(uint8_t addr);

/**
 * @brief  Get effect status
 * @param  effect : expected audio effect
 * @return uint8_t : 0 or 1
 */
uint8_t AudioEffect_effect_status_Get(AUDIOEFFECT_EFFECT_NUM effect);

/**
 * @brief  Onoff effect
 * @param  effect : expected audio effect
 * @param  enable : 0 or 1
 * @return None
 */
void AudioEffect_effect_enable(AUDIOEFFECT_EFFECT_NUM effect, uint8_t enable);

/**
 * @brief  AudioCore source link to Roboeffect source
 * @param  source : AudioCore source enum less than AUDIO_CORE_SOURCE_MAX_NUM
 * @return uint8_t : Roboeffect source index
 */
uint8_t AudioCoreSourceToRoboeffect(int8_t source);

/**
 * @brief  AudioCore sink link to Roboeffect sink
 * @param  sink : AudioCore sink enum less than AUDIO_CORE_SINK_MAX_NUM
 * @return uint8_t : Roboeffect sink index
 */
uint8_t AudioCoreSinkToRoboeffect(int8_t sink);

/**
 * @brief  Get user effect parameters length
 * @param  user_effect_parameters : effect parameters data
 * @return uint16_t : parameters length in Bytes
 */
uint16_t get_user_effect_parameters_len(uint8_t *user_effect_parameters);

/**
 * @brief  Get user effect parameters for roboeffect engine
 * @param  mode : effect mode in enum EFFECT_MODE
 * @return AUDIOEFFECT_EFFECT_PARA : roboeffect parameters
 */
AUDIOEFFECT_EFFECT_PARA * get_user_effect_parameters(uint8_t mode);

/**
 * @brief  Get effect addr in effect flow
 * @param  effect : expected audio effect
 * @return uint8_t : effect addr
 */
uint8_t get_audioeffect_addr(AUDIOEFFECT_EFFECT_NUM effect);

/**
 * @brief  Get audio effect Vol from audioeffectVolArr
 * @param  vol : SDK vol level less than CFG_PARA_MAX_VOLUME_NUM
 * @return uint16_t : gain level which can be set to GainControl
 */
uint16_t get_audioeffectVolArr(uint8_t vol);

/**
 * @brief  Sync some effect parameters when change effect mode
 * @param  void
 * @return None
 */
void AudioEffectParamSync(void);

/**
 * @brief  Check if effect Addr is right
 * @param  addr : expected audio effect addr
 * @return bool : TRUE or FALSE
 */
bool AudioEffect_effectAddr_check(uint8_t addr);

/**
 * @brief  Save effect parameters to flash
 * @param  void
 * @return None
 */
void EffectParamFlashUpdata(void);

/**
 * @brief  Get effect hardware config from flash
 * @param  effectMode : effect mode in enum EFFECT_MODE
 * @param  hw_ct : effect hardware config data pointer
 * @return bool : TRUE or FALSE
 */
bool AudioEffect_GetFlashHwCfg(uint8_t effectMode, HardwareConfigContext *hw_ct);

/**
 * @brief  Get effect parameters from flash
 * @param  effectMode : effect mode in enum EFFECT_MODE
 * @param  hw_ct : effect parameters data pointer
 * @return bool : TRUE or FALSE
 */
bool AudioEffect_GetFlashEffectParam(uint8_t effectMode,  uint8_t *effect_param);

#endif
