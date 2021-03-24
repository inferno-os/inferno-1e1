#ifndef _AUDIO_SOLARIS_H_
#define _AUDIO_SOLARIS_H_
#include <sys/audioio.h>

#define 	Audio_8Bit_Val		8
#define 	Audio_16Bit_Val		16

#define 	Audio_Mono_Val		1
#define 	Audio_Stereo_Val	2

#define 	Audio_Mic_Val		AUDIO_MICROPHONE
#define 	Audio_Linein_Val	AUDIO_LINE_IN

#define		Audio_Speaker_Val	AUDIO_SPEAKER
#define		Audio_Headphone_Val	AUDIO_HEADPHONE
#define		Audio_Lineout_Val	AUDIO_LINE_OUT

#define 	Audio_Pcm_Val		AUDIO_ENCODING_LINEAR
#define 	Audio_Ulaw_Val		AUDIO_ENCODING_ULAW
#define 	Audio_Alaw_Val		AUDIO_ENCODING_ALAW

#define 	Audio_8K_Val		8000
#define 	Audio_11K_Val		11025
#define 	Audio_22K_Val		22050
#define 	Audio_44K_Val		44100

#endif
