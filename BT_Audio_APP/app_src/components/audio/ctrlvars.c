/**
 **************************************************************************************
 * @file    ctrlvars.c
 * @brief   Control Variables Definition
 * 
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include <stdint.h>
#include "debug.h"
#include "app_config.h"
#include "clk.h"
#include "ctrlvars.h"
#include "spi_flash.h"
#include "timeout.h"
#include "delay.h"
#include "breakpoint.h"
#include "nds32_intrinsic.h"
#include "audio_adc.h"
#include "dac.h"
#include "main_task.h"
#include "user_effect_parameter.h"

ControlVariablesContext gCtrlVars;
#ifdef CFG_I2S_SLAVE_TO_SPDIFOUT_EN
SyncModuleContext gSyncModule;
#endif

const int16_t DigVolTab_16[16] =
{
	0,  	332,	460,	541,	636,	748,	880,	1035,  
	1218,	1433,	1686,	1984,	2434,	2946,	3500,	4096
};

const int16_t DigVolTab_32[32] =
{
	0,		31,		36,		42,		49,		58,		67,		78,
	91,		107,	125,	147,	173,	204,	240,	282,
	332,	391,	460,	541,	636,	748,	880,	1035,
	1218,	1433,	1686,	1984,	2434,	2946,	3500,	4096
};

const int16_t DigVolTab_64[64] =
{
      0,    3,    4,    4,    5,    5,    6,    6,    7,    8,    9,   10,   12,   13,   15,   16, 
     18,   21,   23,   26,   29,   33,   36,   41,   46,   52,   58,   65,   73,   82,   92,  103, 
    115,  129,  145,  163,  183,  205,  230,  258,  290,  325,  365,  410,  459,  516,  578,  649, 
    728,  817,  917, 1029, 1154, 1295, 1453, 1630, 1829, 2052, 2303, 2584, 2899, 3253, 3650, 4095, 
};

const uint16_t DigVolTab_256[256] =
{
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001,
    0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002,
    0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003, 0x003,
    0x003, 0x003, 0x003, 0x003, 0x004, 0x004, 0x004, 0x004, 0x004, 0x005, 0x005, 0x005, 0x006, 0x006, 0x006, 0x007,
    0x007, 0x008, 0x008, 0x009, 0x009, 0x00A, 0x00A, 0x00B, 0x00B, 0x00C, 0x00C, 0x00D, 0x00D, 0x00E, 0x00E, 0X00F,
    0x010, 0x011, 0x012, 0x013, 0x014, 0x015, 0x016, 0x017, 0x018, 0x019, 0x01a, 0x01b, 0x01d, 0x01e, 0x01f, 0x021,
    0x022, 0x023, 0x025, 0x027, 0x028, 0x02a, 0x02c, 0x02e, 0x030, 0x032, 0x034, 0x037, 0x039, 0x03c, 0x03e, 0x041,
    0x044, 0x047, 0x04a, 0x04d, 0x051, 0x054, 0x058, 0x05c, 0x060, 0x064, 0x068, 0x06d, 0x072, 0x077, 0x07c, 0x082,
    0x087, 0x08d, 0x093, 0x09a, 0x0a1, 0x0a8, 0x0af, 0x0b7, 0x0bf, 0x0c7, 0x0d0, 0x0d9, 0x0e3, 0x0ed, 0x0f8, 0x102,
    0x10e, 0x11a, 0x126, 0x133, 0x141, 0x14f, 0x15e, 0x16d, 0x17d, 0x18e, 0x1a0, 0x1b2, 0x1c5, 0x1d9, 0x1ee, 0x204,
    0x21a, 0x232, 0x24b, 0x265, 0x280, 0x29c, 0x2ba, 0x2d8, 0x2f9, 0x31a, 0x33d, 0x362, 0x388, 0x3b0, 0x3d9, 0x405,
    0x432, 0x462, 0x493, 0x4c7, 0x4fd, 0x535, 0x570, 0x5ad, 0x5ed, 0x630, 0x676, 0x6bf, 0x70b, 0x75b, 0x7ae, 0x805,
    0x85f, 0x8be, 0x921, 0x988, 0x9f3, 0xa64, 0xad9, 0xb54, 0xbd4, 0xc59, 0xce5, 0xd76, 0xe0e, 0xead, 0xf53, 0xfff
};

const int16_t BassTrebGainTable[16] =
{
    0xf900, //-7dB
    0xfa00, //-6dB
    0xfb00, //-5dB
    0xfc00, //-4dB
    0xfd00, //-3dB
    0xfe00, //-2dB
    0xff00, //-1dB
    0x0000, //+0dB
    0x0100, //+1dB
    0x0200, //+2dB
    0x0300, //+3dB
    0x0400, //+4dB
    0x0500, //+5dB
    0x0600, //+6dB
    0x0700, //+7dB
    0x0800, //+8dB
};

static const uint32_t SupportSampleRateList[13]={
	8000,
    11025,
    12000,
    16000,
    22050,
    24000,
    32000,
    44100,
    48000,
//////i2s//////////////////
    88200,
    96000,
    176400,
    192000,
};

const uint16_t HPCList[3]={
	0xffe, //  48k 20Hz  -1.5db
	0xFFC, //  48k 40Hz  -1.5db
	0xFFD, //  32k 40Hz  -1.5db
};

//EQ style: Classical
const unsigned char Classical[104] = {
    0x00, 0x00,/*pregain*/
	0x00, 0x00,/*calculation_type*/
    0x01, 0x00,/*filter1 enable*/
    0x01, 0x00,/*filter1 type*/
    0x0A, 0x01,/*filter1 f0*/
    0xB7, 0x00,/*filter1 Q*/
    0x28, 0x05,/*filter1 Gain*/
    0x01, 0x00,/*filter2 enable*/
    0x00, 0x00,/*filter2 type*/
    0x7F, 0x02,/*filter2 f0*/
    0x42, 0x03,/*filter2 Q*/
    0x88, 0xFB,/*filter2 Gain*/
    0x01, 0x00,/*filter3 enable*/
    0x00, 0x00,/*filter3 type*/
    0xEE, 0x06,/*filter3 f0*/
    0xD5, 0x02,/*filter3 Q*/
    0x2D, 0xFE,/*filter3 Gain*/
    0x00, 0x00,/*filter4 enable*/
    0x00, 0x00,/*filter4 type*/
    0xE8, 0x03,/*filter4 f0*/
    0xD4, 0x02,/*filter4 Q*/
    0x00, 0x00,/*filter4 Gain*/
    0x01, 0x00,/*filter5 enable*/
    0x02, 0x00,/*filter5 type*/
    0xB2, 0x0E,/*filter5 f0*/
    0x25, 0x01,/*filter5 Q*/
    0x91, 0x04,/*filter5 Gain*/
    0x00, 0x00,/*filter6 enable*/
    0x00, 0x00,/*filter6 type*/
    0xE8, 0x03,/*filter6 f0*/
    0xD4, 0x02,/*filter6 Q*/
    0x00, 0x00,/*filter6 Gain*/
    0x00, 0x00,/*filter7 enable*/
    0x00, 0x00,/*filter7 type*/
    0xE8, 0x03,/*filter7 f0*/
    0xD4, 0x02,/*filter7 Q*/
    0x00, 0x00,/*filter7 Gain*/
    0x00, 0x00,/*filter8 enable*/
    0x00, 0x00,/*filter8 type*/
    0xE8, 0x03,/*filter8 f0*/
    0xD4, 0x02,/*filter8 Q*/
    0x00, 0x00,/*filter8 Gain*/
    0x00, 0x00,/*filter9 enable*/
    0x00, 0x00,/*filter9 type*/
    0xE8, 0x03,/*filter9 f0*/
    0xD4, 0x02,/*filter9 Q*/
    0x00, 0x00,/*filter9 Gain*/
    0x00, 0x00,/*filter10 enable*/
    0x00, 0x00,/*filter10 type*/
    0xE8, 0x03,/*filter10 f0*/
    0xD4, 0x02,/*filter10 Q*/
    0x00, 0x00,/*filter10 Gain*/
};
//EQ style: Vocal Booster
const unsigned char Vocal_Booster[104] = {
    0x00, 0x00,/*pregain*/
	0x00, 0x00,/*calculation_type*/
    0x00, 0x00,/*filter1 enable*/
    0x01, 0x00,/*filter1 type*/
    0x0A, 0x01,/*filter1 f0*/
    0xB7, 0x00,/*filter1 Q*/
    0x28, 0x05,/*filter1 Gain*/
    0x01, 0x00,/*filter2 enable*/
    0x00, 0x00,/*filter2 type*/
    0x60, 0x00,/*filter2 f0*/
    0xFA, 0x01,/*filter2 Q*/
    0x0C, 0xFB,/*filter2 Gain*/
    0x01, 0x00,/*filter3 enable*/
    0x00, 0x00,/*filter3 type*/
    0x1F, 0x02,/*filter3 f0*/
    0x4D, 0x01,/*filter3 Q*/
    0x1A, 0x04,/*filter3 Gain*/
    0x01, 0x00,/*filter4 enable*/
    0x00, 0x00,/*filter4 type*/
    0xA3, 0x09,/*filter4 f0*/
    0x06, 0x03,/*filter4 Q*/
    0x17, 0x01,/*filter4 Gain*/
    0x01, 0x00,/*filter5 enable*/
    0x02, 0x00,/*filter5 type*/
    0xE3, 0x29,/*filter5 f0*/
    0xCD, 0x02,/*filter5 Q*/
    0x44, 0xFE,/*filter5 Gain*/
    0x00, 0x00,/*filter6 enable*/
    0x00, 0x00,/*filter6 type*/
    0xE8, 0x03,/*filter6 f0*/
    0xD4, 0x02,/*filter6 Q*/
    0x00, 0x00,/*filter6 Gain*/
    0x00, 0x00,/*filter7 enable*/
    0x00, 0x00,/*filter7 type*/
    0xE8, 0x03,/*filter7 f0*/
    0xD4, 0x02,/*filter7 Q*/
    0x00, 0x00,/*filter7 Gain*/
    0x00, 0x00,/*filter8 enable*/
    0x00, 0x00,/*filter8 type*/
    0xE8, 0x03,/*filter8 f0*/
    0xD4, 0x02,/*filter8 Q*/
    0x00, 0x00,/*filter8 Gain*/
    0x00, 0x00,/*filter9 enable*/
    0x00, 0x00,/*filter9 type*/
    0xE8, 0x03,/*filter9 f0*/
    0xD4, 0x02,/*filter9 Q*/
    0x00, 0x00,/*filter9 Gain*/
    0x00, 0x00,/*filter10 enable*/
    0x00, 0x00,/*filter10 type*/
    0xE8, 0x03,/*filter10 f0*/
    0xD4, 0x02,/*filter10 Q*/
    0x00, 0x00,/*filter10 Gain*/
};
//EQ style: Flat
const unsigned char Flat[104] = {
    0x00, 0x00,/*pregain*/
	0x00, 0x00,/*calculation_type*/
    0x00, 0x00,/*filter1 enable*/
    0x01, 0x00,/*filter1 type*/
    0x0A, 0x01,/*filter1 f0*/
    0xB7, 0x00,/*filter1 Q*/
    0x28, 0x05,/*filter1 Gain*/
    0x00, 0x00,/*filter2 enable*/
    0x00, 0x00,/*filter2 type*/
    0x60, 0x00,/*filter2 f0*/
    0xFA, 0x01,/*filter2 Q*/
    0x0C, 0xFB,/*filter2 Gain*/
    0x00, 0x00,/*filter3 enable*/
    0x00, 0x00,/*filter3 type*/
    0x1F, 0x02,/*filter3 f0*/
    0x4D, 0x01,/*filter3 Q*/
    0x1A, 0x04,/*filter3 Gain*/
    0x00, 0x00,/*filter4 enable*/
    0x00, 0x00,/*filter4 type*/
    0xA3, 0x09,/*filter4 f0*/
    0x06, 0x03,/*filter4 Q*/
    0x17, 0x01,/*filter4 Gain*/
    0x00, 0x00,/*filter5 enable*/
    0x02, 0x00,/*filter5 type*/
    0xE3, 0x29,/*filter5 f0*/
    0xCD, 0x02,/*filter5 Q*/
    0x44, 0xFE,/*filter5 Gain*/
    0x00, 0x00,/*filter6 enable*/
    0x00, 0x00,/*filter6 type*/
    0xE8, 0x03,/*filter6 f0*/
    0xD4, 0x02,/*filter6 Q*/
    0x00, 0x00,/*filter6 Gain*/
    0x00, 0x00,/*filter7 enable*/
    0x00, 0x00,/*filter7 type*/
    0xE8, 0x03,/*filter7 f0*/
    0xD4, 0x02,/*filter7 Q*/
    0x00, 0x00,/*filter7 Gain*/
    0x00, 0x00,/*filter8 enable*/
    0x00, 0x00,/*filter8 type*/
    0xE8, 0x03,/*filter8 f0*/
    0xD4, 0x02,/*filter8 Q*/
    0x00, 0x00,/*filter8 Gain*/
    0x00, 0x00,/*filter9 enable*/
    0x00, 0x00,/*filter9 type*/
    0xE8, 0x03,/*filter9 f0*/
    0xD4, 0x02,/*filter9 Q*/
    0x00, 0x00,/*filter9 Gain*/
    0x00, 0x00,/*filter10 enable*/
    0x00, 0x00,/*filter10 type*/
    0xE8, 0x03,/*filter10 f0*/
    0xD4, 0x02,/*filter10 Q*/
    0x00, 0x00,/*filter10 Gain*/
};
//EQ style: Pop
const unsigned char Pop[104] = {
    0x00, 0x00,/*pregain*/
	0x00, 0x00,/*calculation_type*/
    0x01, 0x00,/*filter1 enable*/
    0x01, 0x00,/*filter1 type*/
    0xCC, 0x00,/*filter1 f0*/
    0x26, 0x02,/*filter1 Q*/
    0xD7, 0xFD,/*filter1 Gain*/
    0x01, 0x00,/*filter2 enable*/
    0x00, 0x00,/*filter2 type*/
    0xCA, 0x02,/*filter2 f0*/
    0xE8, 0x01,/*filter2 Q*/
    0xF9, 0x03,/*filter2 Gain*/
    0x00, 0x00,/*filter3 enable*/
    0x00, 0x00,/*filter3 type*/
    0x1F, 0x02,/*filter3 f0*/
    0x4D, 0x01,/*filter3 Q*/
    0x1A, 0x04,/*filter3 Gain*/
    0x00, 0x00,/*filter4 enable*/
    0x00, 0x00,/*filter4 type*/
    0xA3, 0x09,/*filter4 f0*/
    0x06, 0x03,/*filter4 Q*/
    0x17, 0x01,/*filter4 Gain*/
    0x01, 0x00,/*filter5 enable*/
    0x02, 0x00,/*filter5 type*/
    0x33, 0x12,/*filter5 f0*/
    0x8C, 0x02,/*filter5 Q*/
    0x0E, 0xFE,/*filter5 Gain*/
    0x00, 0x00,/*filter6 enable*/
    0x00, 0x00,/*filter6 type*/
    0xE8, 0x03,/*filter6 f0*/
    0xD4, 0x02,/*filter6 Q*/
    0x00, 0x00,/*filter6 Gain*/
    0x00, 0x00,/*filter7 enable*/
    0x00, 0x00,/*filter7 type*/
    0xE8, 0x03,/*filter7 f0*/
    0xD4, 0x02,/*filter7 Q*/
    0x00, 0x00,/*filter7 Gain*/
    0x00, 0x00,/*filter8 enable*/
    0x00, 0x00,/*filter8 type*/
    0xE8, 0x03,/*filter8 f0*/
    0xD4, 0x02,/*filter8 Q*/
    0x00, 0x00,/*filter8 Gain*/
    0x00, 0x00,/*filter9 enable*/
    0x00, 0x00,/*filter9 type*/
    0xE8, 0x03,/*filter9 f0*/
    0xD4, 0x02,/*filter9 Q*/
    0x00, 0x00,/*filter9 Gain*/
    0x00, 0x00,/*filter10 enable*/
    0x00, 0x00,/*filter10 type*/
    0xE8, 0x03,/*filter10 f0*/
    0xD4, 0x02,/*filter10 Q*/
    0x00, 0x00,/*filter10 Gain*/
};
//EQ style: Rock
const unsigned char Rock[104] = {
    0x00, 0x00,/*pregain*/
	0x00, 0x00,/*calculation_type*/
    0x01, 0x00,/*filter1 enable*/
    0x01, 0x00,/*filter1 type*/
    0xC4, 0x00,/*filter1 f0*/
    0x84, 0x02,/*filter1 Q*/
    0xD1, 0x04,/*filter1 Gain*/
    0x01, 0x00,/*filter2 enable*/
    0x00, 0x00,/*filter2 type*/
    0xC3, 0x03,/*filter2 f0*/
    0x33, 0x01,/*filter2 Q*/
    0xB9, 0xFD,/*filter2 Gain*/
    0x00, 0x00,/*filter3 enable*/
    0x00, 0x00,/*filter3 type*/
    0x1F, 0x02,/*filter3 f0*/
    0x4D, 0x01,/*filter3 Q*/
    0x1A, 0x04,/*filter3 Gain*/
    0x00, 0x00,/*filter4 enable*/
    0x00, 0x00,/*filter4 type*/
    0xA3, 0x09,/*filter4 f0*/
    0x06, 0x03,/*filter4 Q*/
    0x17, 0x01,/*filter4 Gain*/
    0x01, 0x00,/*filter5 enable*/
    0x02, 0x00,/*filter5 type*/
    0x8F, 0x0D,/*filter5 f0*/
    0x14, 0x02,/*filter5 Q*/
    0x40, 0x04,/*filter5 Gain*/
    0x00, 0x00,/*filter6 enable*/
    0x00, 0x00,/*filter6 type*/
    0xE8, 0x03,/*filter6 f0*/
    0xD4, 0x02,/*filter6 Q*/
    0x00, 0x00,/*filter6 Gain*/
    0x00, 0x00,/*filter7 enable*/
    0x00, 0x00,/*filter7 type*/
    0xE8, 0x03,/*filter7 f0*/
    0xD4, 0x02,/*filter7 Q*/
    0x00, 0x00,/*filter7 Gain*/
    0x00, 0x00,/*filter8 enable*/
    0x00, 0x00,/*filter8 type*/
    0xE8, 0x03,/*filter8 f0*/
    0xD4, 0x02,/*filter8 Q*/
    0x00, 0x00,/*filter8 Gain*/
    0x00, 0x00,/*filter9 enable*/
    0x00, 0x00,/*filter9 type*/
    0xE8, 0x03,/*filter9 f0*/
    0xD4, 0x02,/*filter9 Q*/
    0x00, 0x00,/*filter9 Gain*/
    0x00, 0x00,/*filter10 enable*/
    0x00, 0x00,/*filter10 type*/
    0xE8, 0x03,/*filter10 f0*/
    0xD4, 0x02,/*filter10 Q*/
    0x00, 0x00,/*filter10 Gain*/
};
//EQ style: Jazz
const unsigned char Jazz[104] = {
    0x00, 0x00,/*pregain*/
	0x00, 0x00,/*calculation_type*/
    0x01, 0x00,/*filter1 enable*/
    0x01, 0x00,/*filter1 type*/
    0x70, 0x00,/*filter1 f0*/
    0x85, 0x01,/*filter1 Q*/
    0x2C, 0x04,/*filter1 Gain*/
    0x00, 0x00,/*filter2 enable*/
    0x00, 0x00,/*filter2 type*/
    0xC3, 0x03,/*filter2 f0*/
    0x33, 0x01,/*filter2 Q*/
    0xB9, 0xFD,/*filter2 Gain*/
    0x01, 0x00,/*filter3 enable*/
    0x00, 0x00,/*filter3 type*/
    0x93, 0x02,/*filter3 f0*/
    0x6A, 0x03,/*filter3 Q*/
    0xE4, 0xFD,/*filter3 Gain*/
    0x00, 0x00,/*filter4 enable*/
    0x00, 0x00,/*filter4 type*/
    0xA3, 0x09,/*filter4 f0*/
    0x06, 0x03,/*filter4 Q*/
    0x17, 0x01,/*filter4 Gain*/
    0x01, 0x00,/*filter5 enable*/
    0x02, 0x00,/*filter5 type*/
    0xD8, 0x11,/*filter5 f0*/
    0x25, 0x02,/*filter5 Q*/
    0xC2, 0x02,/*filter5 Gain*/
    0x00, 0x00,/*filter6 enable*/
    0x00, 0x00,/*filter6 type*/
    0xE8, 0x03,/*filter6 f0*/
    0xD4, 0x02,/*filter6 Q*/
    0x00, 0x00,/*filter6 Gain*/
    0x00, 0x00,/*filter7 enable*/
    0x00, 0x00,/*filter7 type*/
    0xE8, 0x03,/*filter7 f0*/
    0xD4, 0x02,/*filter7 Q*/
    0x00, 0x00,/*filter7 Gain*/
    0x00, 0x00,/*filter8 enable*/
    0x00, 0x00,/*filter8 type*/
    0xE8, 0x03,/*filter8 f0*/
    0xD4, 0x02,/*filter8 Q*/
    0x00, 0x00,/*filter8 Gain*/
    0x00, 0x00,/*filter9 enable*/
    0x00, 0x00,/*filter9 type*/
    0xE8, 0x03,/*filter9 f0*/
    0xD4, 0x02,/*filter9 Q*/
    0x00, 0x00,/*filter9 Gain*/
    0x00, 0x00,/*filter10 enable*/
    0x00, 0x00,/*filter10 type*/
    0xE8, 0x03,/*filter10 f0*/
    0xD4, 0x02,/*filter10 Q*/
    0x00, 0x00,/*filter10 Gain*/
};

uint16_t SampleRateIndexGet(uint32_t SampleRate)
{
	volatile uint32_t i;
	for(i = 0; i < sizeof(SupportSampleRateList)/sizeof(SupportSampleRateList[0]); i++)
	{
		if(SampleRate == SupportSampleRateList[i])
		{
			break;
		}
	}
	if(i == 13)
	{
		i =0;
	}
	return i;
}
//系统变量初始化
void CtrlVarsInit(void)
{
	APP_DBG("[SYS]: Loading control vars as default\n");

	//音频系统硬件模块变量初始化
	DefaultParamgsInit();
}

//各个模块默认参数设置函数
void DefaultParamgsInit(void)
{
	memset(&gCtrlVars,  0, sizeof(gCtrlVars));
	//for system control 0x01
	gCtrlVars.AutoRefresh = AutoRefresh_ALL_PARA;

	if(AudioCore.Audioeffect.context_memory)
	{
		memcpy(&gCtrlVars.HwCt, AudioCore.Audioeffect.user_module_parameters, sizeof(gCtrlVars.HwCt));
	}
	else
	{
		AUDIOEFFECT_EFFECT_PARA *para;
		if (mainAppCt.EffectMode  == 0)
		{
#ifdef CFG_AI_DENOISE_EN
            mainAppCt.EffectMode = EFFECT_MODE_MICUSBAI;
#else
#ifdef CFG_FUNC_EFFECT_BYPASS_EN
			mainAppCt.EffectMode = EFFECT_MODE_BYPASS;
#else
#ifdef CFG_FUNC_MIC_KARAOKE_EN
			mainAppCt.EffectMode = EFFECT_MODE_HunXiang;
#else 
			mainAppCt.EffectMode = EFFECT_MODE_MIC;
#endif
#endif
#endif
		}

		para = get_user_effect_parameters(mainAppCt.EffectMode);
		memcpy(&gCtrlVars.HwCt, para->user_module_parameters, sizeof(gCtrlVars.HwCt));
	}
#ifdef CFG_EFFECT_PARAM_IN_FLASH_EN
		if (AudioEffect_GetFlashHwCfg(mainAppCt.EffectMode, &gCtrlVars.HwCt))
		{
			DBG("read HwCt from flash ok\n");
		}
#endif

    //system define
	gCtrlVars.sample_rate		= CFG_PARA_SAMPLE_RATE;
	gCtrlVars.sample_rate_index = SampleRateIndexGet(gCtrlVars.sample_rate);

	//scramble默认开启，设置成POS_NEG
	gCtrlVars.HwCt.DAC0Ct.dac_scramble = POS_NEG + 1;
	//L R数据反，默认交换数据
#if (CFG_CHIP_SEL == CFG_CHIP_BP1524A1) || (CFG_CHIP_SEL == CFG_CHIP_BP1524A2) || (CFG_CHIP_SEL == CFG_CHIP_AP1524A1) || (CFG_CHIP_SEL == CFG_CHIP_BP1564A1) || (CFG_CHIP_SEL == CFG_CHIP_BP1564A2)
	gCtrlVars.HwCt.DAC0Ct.dac_out_mode = MODE1;
#else
	gCtrlVars.HwCt.DAC0Ct.dac_out_mode = MODE0;
#endif
}

void AudioLineSelSet(int8_t ana_input_ch)
{
	//模拟通道先配置为NONE，防止上次配置通道残留，然后再配置需要的模拟通道 

	AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT,  LINEIN_NONE);
	AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN_NONE);

    //AudioADC_AnaInit();
	//AudioADC_VcomConfig(1);//MicBias en
	//AudioADC_MicBias1Enable(1);
	//AudioADC_DynamicElementMatch(ADC0_MODULE, TRUE, TRUE);

	//--------------------line 1-----------------------------------------//
	if(ANA_INPUT_CH_LINEIN1 == ana_input_ch)
	{
		if(gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_en)
		{
			//APP_DBG("LINE 1L En\n");
			AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT,LINEIN1_LEFT);
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT,  LINEIN1_LEFT,  31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_gain);
		}
		if(gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_en)
		{
			//APP_DBG("LINE 1R En\n");
			AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT,LINEIN1_RIGHT);
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT,  31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_gain);
		}
	}

	//--------------------line 2-----------------------------------------//
	if(ANA_INPUT_CH_LINEIN2 == ana_input_ch)
	{
		if(gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_en)
		{
			//APP_DBG("LINE 2L En\n");
			AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT,LINEIN2_LEFT);
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT,  LINEIN2_LEFT,  31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_gain);
		}
		if(gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_en)
		{
			//APP_DBG("LINE 2R En\n");
			AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT,LINEIN2_RIGHT);
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN2_RIGHT,  31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_gain);
		}
	}
}

void AudioLine3MicSelect(void)
{
	if(gCtrlVars.HwCt.ADC1PGACt.pga_mic_enable)
	{
		AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT,MIC_LEFT);
		AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT,  MIC_LEFT, 31 - gCtrlVars.HwCt.ADC1PGACt.pga_mic_gain);//0db bypass
//		gCtrlVars.HwCt.ADC1PGACt.pga_mic_mode = Single;
	}
	else
	{
		AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT,LINEIN_NONE);
	}
}

void AudioAnaChannelSet(int8_t ana_input_ch)
{
	gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_en = 0;
	gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_en = 0;

	if(ANA_INPUT_CH_LINEIN1 == ana_input_ch || ANA_INPUT_CH_LINEIN2 == ana_input_ch)
	{
		gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_en = 1;
		gCtrlVars.HwCt.ADC0PGACt.pga_aux_r_en = 1;
		AudioLineSelSet(ana_input_ch);
	}
}

//音效参数更新之后同步更新模拟Gain和数字Vol
//只更新增益相关参数，其他参数比如通道选择不会同步更新，必须由SDK代码来实现
void AudioCodecGainUpdata(void)
{
//	uint16_t dac_l, dac_r;
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT,  LINEIN1_LEFT,  31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_gain);
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT,  31 - gCtrlVars.HwCt.ADC0PGACt.pga_aux_l_gain);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT,  MIC_LEFT, 31 - gCtrlVars.HwCt.ADC1PGACt.pga_mic_gain);//0db bypass

	AudioADC_HighPassFilterConfig(ADC0_MODULE, HPCList[gCtrlVars.HwCt.ADC0DigitalCt.adc_hpc]);
	AudioADC_HighPassFilterSet(ADC0_MODULE, gCtrlVars.HwCt.ADC0DigitalCt.adc_dc_blocker_en);
	AudioADC_ChannelSwap(ADC0_MODULE, gCtrlVars.HwCt.ADC0DigitalCt.adc_lr_swap);
	AudioADC_VolSet(ADC0_MODULE, gCtrlVars.HwCt.ADC0DigitalCt.adc_dig_l_vol, gCtrlVars.HwCt.ADC0DigitalCt.adc_dig_l_vol);
	AudioADC_ChannelSwap(ADC0_MODULE, gCtrlVars.HwCt.ADC0DigitalCt.adc_mute);

	AudioADC_HighPassFilterConfig(ADC1_MODULE, HPCList[gCtrlVars.HwCt.ADC1DigitalCt.adc_hpc]);
	AudioADC_HighPassFilterSet(ADC1_MODULE, gCtrlVars.HwCt.ADC1DigitalCt.adc_dc_blocker_en);
	AudioADC_ChannelSwap(ADC1_MODULE, gCtrlVars.HwCt.ADC1DigitalCt.adc_lr_swap);
	AudioADC_VolSet(ADC1_MODULE, gCtrlVars.HwCt.ADC1DigitalCt.adc_dig_l_vol, gCtrlVars.HwCt.ADC1DigitalCt.adc_dig_l_vol);
	AudioADC_ChannelSwap(ADC1_MODULE, gCtrlVars.HwCt.ADC1DigitalCt.adc_mute);

	AudioDAC_VolSet(DAC0, gCtrlVars.HwCt.DAC0Ct.dac_dig_l_vol, gCtrlVars.HwCt.DAC0Ct.dac_dig_r_vol);
#ifndef CFG_AUDIO_WIDTH_24BIT
	AudioDAC_DoutModeSet(DAC0, gCtrlVars.HwCt.DAC0Ct.dac_out_mode, WIDTH_16_BIT);
#else
	AudioDAC_DoutModeSet(DAC0, gCtrlVars.HwCt.DAC0Ct.dac_out_mode, WIDTH_24_BIT_2);
#endif
	if(gCtrlVars.HwCt.DAC0Ct.dac_scramble == 0)
	{
		AudioDAC_ScrambleDisable(DAC0);
	}
	else
	{
		AudioDAC_ScrambleEnable(DAC0);
		AudioDAC_ScrambleModeSet(DAC0,(SCRAMBLE_MODULE)gCtrlVars.HwCt.DAC0Ct.dac_scramble - 1);
	}
	if(gCtrlVars.HwCt.DAC0Ct.dac_dither)
	{
		AudioDAC_DitherEnable(DAC0);
	}
	else
	{
		AudioDAC_DitherDisable(DAC0);
	}
}

#ifdef CFG_I2S_SLAVE_TO_SPDIFOUT_EN
__attribute__((section(".driver.isr"))) void Timer4Interrupt(void)
{
	gSyncModule.gSyncTimerCnt++;
	Timer_InterruptFlagClear(SYNC_TIMER_INDEX, UPDATE_INTERRUPT_SRC);
	SyncModule_Get();
}

void SyncModule_Init(void)
{
	GIE_ENABLE();
	NVIC_EnableIRQ(Timer4_IRQn);
	gSyncModule.gSyncTimerCnt = 0;
	gSyncModule.gRefreshFlag = 0;
	gSyncModule.gClearDoneFlag = 0;
	gSyncModule.dat = 0;
	gSyncModule.clkRatio = 4;
	Clock_SyncCtrl_Set(I2S_BCLK_SEL, 1);
	Clock_SyncCtrl_Set(MDAC_MCLK_SEL, 1);
	Timer_Config(SYNC_TIMER_INDEX, SYNC_TIMER_OUT_VALUE, 0);
	Timer_InterrputSrcEnable(SYNC_TIMER_INDEX, UPDATE_INTERRUPT_SRC);
	Timer_Start(SYNC_TIMER_INDEX);
	Clock_SyncCtrl_Set(START_EN, 1);
	Clock_SyncCtrl_Set(UPDATE_DONE_CLR, 1);
}

void SyncModule_Reset(void)
{
	uint32_t SampleRate = I2S_SampleRateGet(CFG_RES_I2S_MODULE);
	if(IsSelectMclkClk1(SampleRate))
	{
		gSyncModule.clkRatio = (AUDIO_PLL_CLK1_FREQ * 4 / I2S_SampleRateGet(CFG_RES_I2S_MODULE)) / 64;
	}
	else
	{
		gSyncModule.clkRatio = (AUDIO_PLL_CLK2_FREQ * 4 / I2S_SampleRateGet(CFG_RES_I2S_MODULE)) / 64;
	}
	DBG("BCLK_MCLK_RATIO = %d\n", gSyncModule.clkRatio);
	Clock_SyncCtrl_Set(I2S_BCLK_SEL, 0);
	Clock_SyncCtrl_Set(MDAC_MCLK_SEL, 0);
	Clock_SyncCtrl_Set(START_EN, 0);
	Clock_SyncCtrl_Set(UPDATE_DONE_CLR, 0);
	gSyncModule.gSyncTimerCnt = 0;
	gSyncModule.gRefreshFlag = 0;
	gSyncModule.gClearDoneFlag = 0;
	gSyncModule.dat = 0;
	Clock_SyncCtrl_Set(I2S_BCLK_SEL, 1);
	Clock_SyncCtrl_Set(MDAC_MCLK_SEL, 1);
	Clock_SyncCtrl_Set(START_EN, 1);
	Clock_SyncCtrl_Set(UPDATE_DONE_CLR, 1);
}

void SyncModule_Get(void)
{
	if(gSyncModule.gRefreshFlag)//上次处理还没有处理完，这次不再使用，准备使用下次
		return;

	//如果再次进来后，还是没清除，则这次结果不用
//	if(SREG_CLK_SYNC_CTRL.CLK_SYNC_CNT_UPDATE_DONE)
//	{
//		gClearDoneFlag = 1;
//		return;
//	}
	Clock_SyncCtrl_Set(UPDATE_EN, 1);
	WaitUs(3);
	gSyncModule.gI2sBclkCnt = Clock_ClkCnt_Get(I2S_BCLK_CNT1);
	gSyncModule.gI2sBclkCnt = (gSyncModule.gI2sBclkCnt << 32)  + Clock_ClkCnt_Get(I2S_BCLK_CNT0);
	gSyncModule.gDacMclkCnt = Clock_ClkCnt_Get(MDAC_MCLK_CNT1);
	gSyncModule.gDacMclkCnt = (gSyncModule.gDacMclkCnt << 32)  + Clock_ClkCnt_Get(MDAC_MCLK_CNT0);
	gSyncModule.gRefreshFlag = 1;
}

void SyncModule_Process(void)
{
	if(gSyncModule.gRefreshFlag || gSyncModule.gClearDoneFlag)
	{
		uint16_t defVal;

		if(gSyncModule.gRefreshFlag)
		{
			gSyncModule.gMClkFromBCLK = gSyncModule.gI2sBclkCnt  * 1000000 / SYNC_TIMER_OUT_VALUE * gSyncModule.clkRatio / gSyncModule.gSyncTimerCnt;
			gSyncModule.gMClkFromDPLL = gSyncModule.gDacMclkCnt  * 1000000 / SYNC_TIMER_OUT_VALUE / gSyncModule.gSyncTimerCnt;
//			DBG("gMClkFromBCLK = %f, gMClkFromDPLL = %f,  ", gSyncModule.gMClkFromBCLK*1.0, gSyncModule.gMClkFromDPLL*1.0);

			if(gSyncModule.gMClkFromBCLK > gSyncModule.gMClkFromDPLL)
			{
				defVal = gSyncModule.gMClkFromBCLK - gSyncModule.gMClkFromDPLL;
				if(defVal > 100)
				{
					gSyncModule.dat = 10;
					*(uint32_t *)0x40026008 += gSyncModule.dat;//defVal/10 * 5 + 1;
				}
				else
				{
					gSyncModule.dat = 5;
					if(defVal > 50)
					{
						if(defVal <= gSyncModule.defVal_bak)
						{
							gSyncModule.dat = 0;
						}
					}
					else if(defVal < 50)
					{
						if(defVal <= gSyncModule.defVal_bak)
						{
							gSyncModule.dat = 0;
						}
						else
						{
							gSyncModule.dat = 1;
						}
					}
					*(uint32_t *)0x40026008 += gSyncModule.dat;
				}

				gSyncModule.defVal_bak = defVal;
				//DBG("+%d, +%d, 0x%08x\n", defVal, defVal > 50?10:1, *(uint32_t *)0x40026008);
//				DBG("+%d, +%d, 0x%08x\n", defVal, gSyncModule.dat, *(uint32_t *)0x40026008);
			}
			else if(gSyncModule.gMClkFromBCLK < gSyncModule.gMClkFromDPLL)
			{
				defVal = gSyncModule.gMClkFromDPLL - gSyncModule.gMClkFromBCLK;
				gSyncModule.dat = 0;
				if(defVal > 100)
				{
					gSyncModule.dat = 10;
					*(uint32_t *)0x40026008 -= gSyncModule.dat;//defVal/10 * 5 + 1;
				}
				else
				{
					gSyncModule.dat = 5;
					if(defVal > 50)
					{
						if(defVal <= gSyncModule.defVal_bak)
						{
							gSyncModule.dat = 0;
						}
					}
					else if(defVal < 50)
					{
						if(defVal <= gSyncModule.defVal_bak)
						{
							gSyncModule.dat = 0;
						}
						else
						{
							gSyncModule.dat = 1;
						}
					}
					*(uint32_t *)0x40026008 -= gSyncModule.dat;
				}
				gSyncModule.defVal_bak = defVal;
				//DBG("-%d, -%d, 0x%08x\n", defVal, defVal > 50?10:1, *(uint32_t *)0x40026008);
//				DBG("-%d, -%d, 0x%08x\n", defVal, gSyncModule.dat, *(uint32_t *)0x40026008);
			}
		}

//		DBG("\n");
		gSyncModule.gRefreshFlag = 0;
		gSyncModule.gClearDoneFlag = 0;
	}
}
#endif
