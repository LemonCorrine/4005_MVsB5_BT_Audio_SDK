#ifndef _ROBOEFFECT_CONFIG_H_
#define _ROBOEFFECT_CONFIG_H_


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus


#include <stdio.h>
#include <nds32_intrinsic.h>
#include "type.h"
#include "roboeffect_api.h"
#include "audio_effect_library.h"


#define AUTO_TUNE_ENABLE                   (1)
#define DC_BLOCKER_ENABLE                  (0)
#define DRC_ENABLE                         (1)
#define ECHO_ENABLE                        (1)
#define EQ_ENABLE                          (1)
#define NOISE_SUPPRESSOR_EXPANDER_ENABLE   (1)
#define FREQ_SHIFTER_ENABLE                (1)
#define HOWLING_SUPPRESSOR_ENABLE          (1)
#define NOISE_GATE_ENABLE                  (0)
#define PITCH_SHIFTER_ENABLE               (1)
#define REVERB_ENABLE                      (1)
#define SILENCE_DETECTOR_ENABLE            (1)
#define THREE_D_ENABLE                     (1)
#define VIRTUAL_BASS_ENABLE                (1)
#define VOICE_CHANGER_ENABLE               (1)
#define GAIN_CONTROL_ENABLE                (1)
#define VOCAL_CUT_ENABLE                   (0)//(1)
#define REVERB_PRO_ENABLE                  (0)
#define VOICE_CHANGER_PRO_ENABLE           (1)
#define PHASE_CONTROL_ENABLE               (0)//(1)
#define VOCAL_REMOVER_ENABLE               (0)
#define PITCH_SHIFTER_PRO_ENABLE           (1)
#define VIRTUAL_BASS_CLASSIC_ENABLE        (0)
#define PCM_DELAY_ENABLE                   (1)
#define HARMONIC_EXCITER_ENABLE            (1)
#define CHORUS_ENABLE                      (0)
#define AUTO_WAH_ENABLE                    (0)
#define STEREO_WIDENER_ENABLE              (1)
#define PINGPONG_ENABLE                    (0)
#define THREE_D_PLUS_ENABLE                (0)
#define SINE_GENERATOR_ENABLE              (0)
#define NOISE_SUPPRESSOR_BLUE_ENABLE       (0)
#define FLANGER_ENABLE                     (0)
#define FREQ_SHIFTER_FINE_ENABLE           (0)
#define OVER_DRIVE_ENABLE                  (0)
#define DISTORTION_DS1_ENABLE              (0)
#define EQ_DRC_ENABLE                      (1)
#define AEC_ENABLE                         (1)
#define COMPANDER_ENABLE                   (1)
#define LOW_LEVEL_COMPRESSOR_ENABLE        (1)
#define BEAT_TRACKER_ENABLE                (0)
#define ENGINE_SOUND_ENABLE                (0)
#define BIQUAD_ENABLE                      (0)
#define CHORUS2_ENABLE                     (0)
#define REVERB_PLATE_ENABLE                (1)
#define HOWLING_SUPPRESSOR_FINE_ENABLE     (1)
#define HOWLING_GUARD_ENABLE               (0)
#define VIRTUAL_BASS_TD_ENABLE             (0)
#define FADER_ENABLE                       (1)//do not close as it is a node effect
#define DOWNMIX_2TO1_ENABLE                (1)//do not close as it is a node effect
#define UPMIX_1TO2_ENABLE                  (1)//do not close as it is a node effect
#define CHANNEL_COMBINER_ENABLE            (1)//do not close as it is a node effect
#define CHANNEL_SELECTOR_ENABLE            (1)//do not close as it is a node effect
#define ROUTE_SELECTOR_ENABLE              (1)//do not close as it is a node effect
#define AI_DENOISE_ENABLE                  (0)
#define USER_GAIN_ENABLE                   (0)

//effects parameters length
#define ROBOEFFECT_AUTO_TUNE_PARAM_LEN                3
#define ROBOEFFECT_DC_BLOCKER_PARAM_LEN               0
#define ROBOEFFECT_DRC_PARAM_LEN                      26
#define ROBOEFFECT_ECHO_PARAM_LEN                     7
#define ROBOEFFECT_EQ_PARAM_LEN                       52
#define ROBOEFFECT_NOISE_SUPPRESSOR_EXPANDER_PARAM_LEN 4
#define ROBOEFFECT_FREQ_SHIFTER_PARAM_LEN             1
#define ROBOEFFECT_HOWLING_SUPPRESSOR_PARAM_LEN       1
#define ROBOEFFECT_NOISE_GATE_PARAM_LEN               5
#define ROBOEFFECT_PITCH_SHIFTER_PARAM_LEN            1
#define ROBOEFFECT_REVERB_PARAM_LEN                   6
#define ROBOEFFECT_SILENCE_DETECTOR_PARAM_LEN         1
#define ROBOEFFECT_THREE_D_PARAM_LEN                  1
#define ROBOEFFECT_VIRTUAL_BASS_PARAM_LEN             3
#define ROBOEFFECT_VOICE_CHANGER_PARAM_LEN            2
#define ROBOEFFECT_GAIN_CONTROL_PARAM_LEN             2
#define ROBOEFFECT_VOCAL_CUT_PARAM_LEN                1
#define ROBOEFFECT_REVERB_PRO_PARAM_LEN               16
#define ROBOEFFECT_VOICE_CHANGER_PRO_PARAM_LEN        2
#define ROBOEFFECT_PHASE_CONTROL_PARAM_LEN            1
#define ROBOEFFECT_VOCAL_REMOVER_PARAM_LEN            2
#define ROBOEFFECT_PITCH_SHIFTER_PRO_PARAM_LEN        1
#define ROBOEFFECT_VIRTUAL_BASS_CLASSIC_PARAM_LEN     2
#define ROBOEFFECT_PCM_DELAY_PARAM_LEN                3
#define ROBOEFFECT_HARMONIC_EXCITER_PARAM_LEN         3
#define ROBOEFFECT_CHORUS_PARAM_LEN                   6
#define ROBOEFFECT_AUTO_WAH_PARAM_LEN                 6
#define ROBOEFFECT_STEREO_WIDENER_PARAM_LEN           1
#define ROBOEFFECT_PINGPONG_PARAM_LEN                 5
#define ROBOEFFECT_THREE_D_PLUS_PARAM_LEN             1
#define ROBOEFFECT_SINE_GENERATOR_PARAM_LEN           5
#define ROBOEFFECT_NOISE_SUPPRESSOR_BLUE_PARAM_LEN    1
#define ROBOEFFECT_FLANGER_PARAM_LEN                  6
#define ROBOEFFECT_FREQ_SHIFTER_FINE_PARAM_LEN        1
#define ROBOEFFECT_OVER_DRIVE_PARAM_LEN               1
#define ROBOEFFECT_DISTORTION_DS1_PARAM_LEN           2
#define ROBOEFFECT_EQ_DRC_PARAM_LEN                   78
#define ROBOEFFECT_AEC_PARAM_LEN                      1
#define ROBOEFFECT_COMPANDER_PARAM_LEN                6
#define ROBOEFFECT_LOW_LEVEL_COMPRESSOR_PARAM_LEN     4
#define ROBOEFFECT_BEAT_TRACKER_PARAM_LEN             3
#define ROBOEFFECT_ENGINE_SOUND_PARAM_LEN             5
#define ROBOEFFECT_BIQUAD_PARAM_LEN                   5
#define ROBOEFFECT_CHORUS2_PARAM_LEN                  8
#define ROBOEFFECT_REVERB_PLATE_PARAM_LEN             7
#define ROBOEFFECT_HOWLING_SUPPRESSOR_FINE_PARAM_LEN  2
#define ROBOEFFECT_HOWLING_GUARD_PARAM_LEN            7
#define ROBOEFFECT_VIRTUAL_BASS_TD_PARAM_LEN          3
#define ROBOEFFECT_OVER_DRIVE_PLOY_PARAM_LEN          2
#define ROBOEFFECT_FADER_PARAM_LEN                    1
#define ROBOEFFECT_DOWNMIX_2TO1_PARAM_LEN             0
#define ROBOEFFECT_UPMIX_1TO2_PARAM_LEN               0
#define ROBOEFFECT_CHANNEL_COMBINER_PARAM_LEN         0
#define ROBOEFFECT_CHANNEL_SELECTOR_PARAM_LEN         1
#define ROBOEFFECT_ROUTE_SELECTOR_PARAM_LEN           1
#define ROBOEFFECT_AI_DENOISE_PARAM_LEN               1
#define ROBOEFFECT_USER_GAIN_PARAM_LEN                1


//null interface
bool roboeffect_null_init_if(void *node);
bool roboeffect_null_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_null_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_null_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//auto_tune interface
bool roboeffect_auto_tune_init_if(void *node);
bool roboeffect_auto_tune_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_auto_tune_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_auto_tune_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//dc_blocker interface
bool roboeffect_dc_blocker_init_if(void *node);
bool roboeffect_dc_blocker_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_dc_blocker_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_dc_blocker_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//drc interface
bool roboeffect_drc_init_if(void *node);
bool roboeffect_drc_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_drc_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_drc_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//echo interface
bool roboeffect_echo_init_if(void *node);
bool roboeffect_echo_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_echo_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_echo_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//eq interface
bool roboeffect_eq_init_if(void *node);
bool roboeffect_eq_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_eq_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_eq_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//noise_suppressor_expander interface
bool roboeffect_noise_suppressor_expander_init_if(void *node);
bool roboeffect_noise_suppressor_expander_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_noise_suppressor_expander_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_noise_suppressor_expander_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//freq_shifter interface
bool roboeffect_freq_shifter_init_if(void *node);
bool roboeffect_freq_shifter_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_freq_shifter_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_freq_shifter_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//howling_suppressor interface
bool roboeffect_howling_suppressor_init_if(void *node);
bool roboeffect_howling_suppressor_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_howling_suppressor_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_howling_suppressor_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//noise_gate interface
bool roboeffect_noise_gate_init_if(void *node);
bool roboeffect_noise_gate_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_noise_gate_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_noise_gate_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//pitch_shifter interface
bool roboeffect_pitch_shifter_init_if(void *node);
bool roboeffect_pitch_shifter_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_pitch_shifter_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_pitch_shifter_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//reverb interface
bool roboeffect_reverb_init_if(void *node);
bool roboeffect_reverb_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_reverb_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_reverb_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//silence_detector interface
bool roboeffect_silence_detector_init_if(void *node);
bool roboeffect_silence_detector_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_silence_detector_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_silence_detector_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//three_d interface
bool roboeffect_three_d_init_if(void *node);
bool roboeffect_three_d_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_three_d_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_three_d_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//virtual_bass interface
bool roboeffect_virtual_bass_init_if(void *node);
bool roboeffect_virtual_bass_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_virtual_bass_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_virtual_bass_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//voice_changer interface
bool roboeffect_voice_changer_init_if(void *node);
bool roboeffect_voice_changer_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_voice_changer_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_voice_changer_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//gain_control interface
bool roboeffect_gain_control_init_if(void *node);
bool roboeffect_gain_control_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_gain_control_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_gain_control_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//vocal_cut interface
bool roboeffect_vocal_cut_init_if(void *node);
bool roboeffect_vocal_cut_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_vocal_cut_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_vocal_cut_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//reverb_pro interface
bool roboeffect_reverb_pro_init_if(void *node);
bool roboeffect_reverb_pro_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_reverb_pro_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_reverb_pro_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//voice_changer_pro interface
bool roboeffect_voice_changer_pro_init_if(void *node);
bool roboeffect_voice_changer_pro_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_voice_changer_pro_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_voice_changer_pro_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//phase_control interface
bool roboeffect_phase_control_init_if(void *node);
bool roboeffect_phase_control_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_phase_control_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_phase_control_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//vocal_remover interface
bool roboeffect_vocal_remover_init_if(void *node);
bool roboeffect_vocal_remover_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_vocal_remover_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_vocal_remover_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//pitch_shifter_pro interface
bool roboeffect_pitch_shifter_pro_init_if(void *node);
bool roboeffect_pitch_shifter_pro_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_pitch_shifter_pro_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_pitch_shifter_pro_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//virtual_bass_classic interface
bool roboeffect_virtual_bass_classic_init_if(void *node);
bool roboeffect_virtual_bass_classic_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_virtual_bass_classic_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_virtual_bass_classic_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//pcm_delay interface
bool roboeffect_pcm_delay_init_if(void *node);
bool roboeffect_pcm_delay_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_pcm_delay_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_pcm_delay_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//harmonic_exciter interface
bool roboeffect_harmonic_exciter_init_if(void *node);
bool roboeffect_harmonic_exciter_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_harmonic_exciter_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_harmonic_exciter_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//chorus interface
bool roboeffect_chorus_init_if(void *node);
bool roboeffect_chorus_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_chorus_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_chorus_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//auto_wah interface
bool roboeffect_auto_wah_init_if(void *node);
bool roboeffect_auto_wah_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_auto_wah_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_auto_wah_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//stereo_widener interface
bool roboeffect_stereo_widener_init_if(void *node);
bool roboeffect_stereo_widener_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_stereo_widener_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_stereo_widener_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//pingpong interface
bool roboeffect_pingpong_init_if(void *node);
bool roboeffect_pingpong_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_pingpong_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_pingpong_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//three_d_plus interface
bool roboeffect_three_d_plus_init_if(void *node);
bool roboeffect_three_d_plus_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_three_d_plus_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_three_d_plus_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//sine_generator interface
bool roboeffect_sine_generator_init_if(void *node);
bool roboeffect_sine_generator_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_sine_generator_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_sine_generator_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//noise_suppressor_blue interface
bool roboeffect_noise_suppressor_blue_init_if(void *node);
bool roboeffect_noise_suppressor_blue_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_noise_suppressor_blue_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_noise_suppressor_blue_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//flanger interface
bool roboeffect_flanger_init_if(void *node);
bool roboeffect_flanger_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_flanger_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_flanger_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//freq_shifter_fine interface
bool roboeffect_freq_shifter_fine_init_if(void *node);
bool roboeffect_freq_shifter_fine_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_freq_shifter_fine_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_freq_shifter_fine_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//over_drive interface
bool roboeffect_over_drive_init_if(void *node);
bool roboeffect_over_drive_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_over_drive_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_over_drive_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//distortion_ds1 interface
bool roboeffect_distortion_ds1_init_if(void *node);
bool roboeffect_distortion_ds1_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_distortion_ds1_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_distortion_ds1_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//eq_drc interface
bool roboeffect_eq_drc_init_if(void *node);
bool roboeffect_eq_drc_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_eq_drc_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_eq_drc_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//aec interface
bool roboeffect_aec_init_if(void *node);
bool roboeffect_aec_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_aec_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_aec_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//compander interface
bool roboeffect_compander_init_if(void *node);
bool roboeffect_compander_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_compander_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_compander_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//low_level_compressor interface
bool roboeffect_low_level_compressor_init_if(void *node);
bool roboeffect_low_level_compressor_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_low_level_compressor_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_low_level_compressor_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//beat_tracker interface
bool roboeffect_beat_tracker_init_if(void *node);
bool roboeffect_beat_tracker_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_beat_tracker_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_beat_tracker_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//engine_sound interface
bool roboeffect_engine_sound_init_if(void *node);
bool roboeffect_engine_sound_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_engine_sound_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_engine_sound_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//biquad interface
bool roboeffect_biquad_init_if(void *node);
bool roboeffect_biquad_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_biquad_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_biquad_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//chorus2 interface
bool roboeffect_chorus2_init_if(void *node);
bool roboeffect_chorus2_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_chorus2_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_chorus2_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//reverb_plate interface
bool roboeffect_reverb_plate_init_if(void *node);
bool roboeffect_reverb_plate_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_reverb_plate_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_reverb_plate_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//howling_suppressor_fine interface
bool roboeffect_howling_suppressor_fine_init_if(void *node);
bool roboeffect_howling_suppressor_fine_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_howling_suppressor_fine_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_howling_suppressor_fine_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//howling_guard interface
bool roboeffect_howling_guard_init_if(void *node);
bool roboeffect_howling_guard_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_howling_guard_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_howling_guard_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//virtual_bass_td interface
bool roboeffect_virtual_bass_td_init_if(void *node);
bool roboeffect_virtual_bass_td_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_virtual_bass_td_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_virtual_bass_td_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//over_drive_ploy interface
bool roboeffect_over_drive_ploy_init_if(void *node);
bool roboeffect_over_drive_ploy_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_over_drive_ploy_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_over_drive_ploy_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//fader interface
bool roboeffect_fader_init_if(void *node);
bool roboeffect_fader_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_fader_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_fader_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//downmix_2to1 interface
bool roboeffect_downmix_2to1_init_if(void *node);
bool roboeffect_downmix_2to1_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_downmix_2to1_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_downmix_2to1_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//upmix_1to2 interface
bool roboeffect_upmix_1to2_init_if(void *node);
bool roboeffect_upmix_1to2_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_upmix_1to2_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_upmix_1to2_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//channel_combiner interface
bool roboeffect_channel_combiner_init_if(void *node);
bool roboeffect_channel_combiner_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_channel_combiner_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_channel_combiner_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//channel_selector interface
bool roboeffect_channel_selector_init_if(void *node);
bool roboeffect_channel_selector_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_channel_selector_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_channel_selector_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//route_selector interface
bool roboeffect_route_selector_init_if(void *node);
bool roboeffect_route_selector_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_route_selector_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_route_selector_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//ai_denoise interface
bool roboeffect_ai_denoise_init_if(void *node);
bool roboeffect_ai_denoise_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_ai_denoise_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_ai_denoise_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//user_gain interface
bool roboeffect_user_gain_init_if(void *node);
bool roboeffect_user_gain_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len);
bool roboeffect_user_gain_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n);
int32_t roboeffect_user_gain_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters);

//user callback
uint16_t effect_property_for_robo_size(void);
roboeffect_effect_property_struct *effect_property_for_robo_get(void);
char *effect_lib_version_return(void);
//helper api
int16_t *roboeffect_get_param_by_raw_data(uint8_t addr, uint8_t *enable, uint8_t *len, const uint8_t *parameters);


#ifdef __cplusplus
}
#endif//__cplusplus
#endif/*_ROBOEFFECT_CONFIG_H_*/
