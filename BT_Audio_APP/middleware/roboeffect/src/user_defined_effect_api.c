/**
 **************************************************************************************
 * @file    user_defined_api.c
 * @brief   interface for user defined effect algorithm
 *
 * @author  Castle Cai
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <nds32_utils_math.h>
#include "roboeffect_api.h"
#include "user_defined_effect_api.h"
#include "nn_denoise_api.h"

/**
 * ai denoise begin
*/
bool roboeffect_ai_denoise_init_if(void *node)
{
	uint8_t *context_ptr, *scratch;
	roboeffect_user_defined_effect_info info;
	ai_denoise_struct *ai_info;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct
	*/
	ai_info = info.context_memory;

	/**
	 * clear user defined algorithm context
	*/
	memset(ai_info, 0x00, sizeof(ai_denoise_struct) + AI_MEMORY_SIZE + AI_RAM_SIZE);

	/**
	 *get user defined algorithm context
	*/
	context_ptr = ((uint8_t*)info.context_memory) + sizeof(ai_denoise_struct);

	/**
	 * get scratch memory
	*/
	scratch = info.scratch_memory;

	/**
	 * initialize 
	*/
	ai_info->p_nn_denoise = nn_denoise_init(context_ptr, scratch, info.sample_rate, AI_FRAME_TIME, NN_MODE_DENOISE_M3);
	nn_denoise_set_ram(ai_info->p_nn_denoise, context_ptr + AI_MEMORY_SIZE);

	/**
	 * set default parameters
	*/
	nn_set_max_suppress(ai_info->p_nn_denoise, (float)info.parameters[0]);

	ai_info->block_size = info.sample_rate * AI_FRAME_TIME / 1000;

	return TRUE;
}

bool roboeffect_ai_denoise_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len)
{
	uint8_t method_flag = 0;
	roboeffect_user_defined_effect_info info;
	ai_denoise_struct *ai_info;

	/**
	 * check parameters and update to effect instance
	*/
	if(ROBOEFFECT_ERROR_OK > roboeffect_user_defined_params_check(node, new_param, param_num, len, &method_flag))
	{
		return FALSE;
	}

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct
	*/
	ai_info = info.context_memory;

	if((method_flag & METHOD_CFG_1) && info.is_active)
	{
		// printf("call METHOD_CFG_1 func: %d\n", info.parameters[0]);
		nn_set_max_suppress(ai_info->p_nn_denoise, (float)info.parameters[0]);
	}

	return TRUE;
}


bool roboeffect_ai_denoise_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n)
{
	roboeffect_user_defined_effect_info info;
	ai_denoise_struct *ai_info;
	int32_t block, clip_size;
	
	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct
	*/
	ai_info = info.context_memory;
	
	/**
	 * apply data
	*/
	clip_size = ai_info->block_size;
	block = n / clip_size;
	if(block > 0)
	{
		while(block--)
		{
			nds32_convert_q15_f32(pcm_in1, ai_info->float_buffer_in, clip_size);
			nn_denoise_process(ai_info->p_nn_denoise, ai_info->float_buffer_in, ai_info->float_buffer_out);
			nds32_convert_f32_q15(ai_info->float_buffer_out, pcm_out, clip_size);

			pcm_in1 += clip_size;
			pcm_out += clip_size;
			// printf("%d, ", block);
		}
	}

	return TRUE;
}


int32_t roboeffect_ai_denoise_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters)
{
	return AI_SCRATCH_SIZE;
}

int32_t roboeffect_user_gain_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters)
{
	return 16;
}

/**
 * user gain begin
*/
bool roboeffect_user_gain_init_if(void *node)
{
	roboeffect_user_defined_effect_info info;
	user_gain_struct *gain_info;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
	gain_info = info.context_memory;

	/**
	 * dummy initialize the context of the user_gain
	*/
	gain_info->data_a = 0xAB;
	gain_info->data_b = 0xCD;


	return TRUE;
}

bool roboeffect_user_gain_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len)
{
	uint8_t method_flag = 0;
	roboeffect_user_defined_effect_info info;
//	user_gain_struct *gain_info;

	/**
	 * check parameters and update to effect instance
	*/
	if(ROBOEFFECT_ERROR_OK < roboeffect_user_defined_params_check(node, new_param, param_num, len, &method_flag))
	{
		return FALSE;
	}

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
//	gain_info = info.context_memory;

	if((method_flag & METHOD_CFG_1) && info.is_active)
	{

	}

	return TRUE;
}

#define SCALING_Q12_MAX (0x1000)
static inline uint32_t db_to_scaling(float db, uint32_t scaling_max)
{
	return (uint32_t)roundf(powf(10.0f,((float)db/20.0f)) * scaling_max);
}

bool roboeffect_user_gain_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n)
{
	int s;
	int32_t pregain, *pcm_in_24 = (int32_t*)pcm_in1, *pcm_out_24 = (int32_t*)pcm_out;
	roboeffect_user_defined_effect_info info;
//	user_gain_struct *gain_info;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
//	gain_info = info.context_memory;
	
	/**
	 * apply data
	*/

	//info.parameters[0] is a dB, convert it to Q12.
	
	pregain = db_to_scaling(((float)info.parameters[0])/100, SCALING_Q12_MAX);

	if(info.width == BITS_24)
	{
		for(s = 0; s < n; s++)
		{
			if(info.ch_num == 2)
			{
				pcm_out_24[2 * s + 0] = __nds32__clips((((int64_t)pcm_in_24[2 * s + 0] * pregain + 2048) >> 12), (24)-1);
				pcm_out_24[2 * s + 1] = __nds32__clips((((int64_t)pcm_in_24[2 * s + 1] * pregain + 2048) >> 12), (24)-1);
			}
			else
			{
				pcm_out_24[s] = __nds32__clips((((int64_t)pcm_in_24[s] * pregain + 2048) >> 12), (24)-1);
			}
		}
	}
	else if(info.width == BITS_16)
	{
		for(s = 0; s < n; s++)
		{
			if(info.ch_num == 2)
			{
				pcm_out[2 * s + 0] = __nds32__clips((((int32_t)pcm_in1[2 * s + 0] * pregain + 2048) >> 12), (16)-1);
				pcm_out[2 * s + 1] = __nds32__clips((((int32_t)pcm_in1[2 * s + 1] * pregain + 2048) >> 12), (16)-1);
			}
			else
			{
				pcm_out[s] = __nds32__clips((((int32_t)pcm_in1[s] * pregain + 2048) >> 12), (16)-1);
			}
		}
	}

	return TRUE;
}



#if 0
/**
 * howling_supressor_fine begin
*/
bool roboeffect_howling_suppressor_fine_init_if(void *node)
{
	uint8_t *context_ptr;
	roboeffect_user_defined_effect_info info;
	HowlingFineContext *hfc;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
	hfc = info.context_memory;

	/**
	 * dummy initialize the context of the user_gain
	*/
	if(HOWLING_SUPPRESSOR_FINE_ERROR_OK == howling_suppressor_fine_init(hfc, info.sample_rate))
		return TRUE;
	else
		return FALSE;

	return TRUE;
}

bool roboeffect_howling_suppressor_fine_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len)
{
	int ret;
	uint8_t method_flag = 0;
	roboeffect_user_defined_effect_info info;
	HowlingFineContext *hfc;

	/**
	 * check parameters and update to effect instance
	*/
	if(ROBOEFFECT_ERROR_OK < roboeffect_user_defined_params_check(node, new_param, param_num, len, &method_flag))
	{
		return FALSE;
	}

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
	hfc = info.context_memory;

	if((method_flag & METHOD_CFG_1) && info.is_active)
	{

	}

	return TRUE;
}

bool roboeffect_howling_suppressor_fine_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n)
{
	int i, s;
	int32_t pregain, *pcm_in_24 = (int32_t*)pcm_in1, *pcm_out_24 = (int32_t*)pcm_out;
	roboeffect_user_defined_effect_info info;
	HowlingFineContext *hfc;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
	hfc = info.context_memory;
	
	/**
	 * apply data
	*/
	// printf("%d, %d, %d, %d\n", info.parameters[0], info.parameters[1], (int32_t)info.parameters[0] * 1024 / 100, (int32_t)info.parameters[1] * 1024 / 100);
	howling_suppressor_fine_apply(hfc, pcm_in1, pcm_out, n, (int32_t)info.parameters[0] * 1024 / 100, (int32_t)info.parameters[1] * 1024 / 100);


	return TRUE;
}

#endif
