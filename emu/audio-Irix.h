#ifndef _AUDIO_IRIX_H_
#define _AUDIO_IRIX_H_
#include 	<dmedia/audio.h>
#include	"audio.h"
#include	"svp.h"

#define 	Audio_8Bit_Val		AL_SAMPLE_8
#define 	Audio_16Bit_Val		AL_SAMPLE_16

#define 	Audio_Mono_Val		AL_MONO
#define 	Audio_Stereo_Val	AL_STEREO

#define 	Audio_Mic_Val		AL_INPUT_MIC
#define 	Audio_Linein_Val	AL_INPUT_LINE

#define		Audio_Speaker_Val	0
#define		Audio_Headphone_Val	(Audio_Speaker_Val+1)
#define		Audio_Lineout_Val	(Audio_Speaker_Val+2)

#define 	Audio_Pcm_Val		AL_SAMPFMT_TWOSCOMP
#define 	Audio_Ulaw_Val		(Audio_Pcm_Val+1)
#define 	Audio_Alaw_Val		(Audio_Pcm_Val+2)

#define 	Audio_8K_Val		AL_RATE_8000
#define 	Audio_11K_Val		AL_RATE_11025
#define 	Audio_22K_Val		AL_RATE_22050
#define 	Audio_44K_Val		AL_RATE_44100

#define		Bits_Per_Byte		8

#endif
