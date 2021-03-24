#ifndef _AUDIO_NT_H_
#define _AUDIO_NT_H_

#include <windows.h>
#include	"lib9.h"
#include	"dat.h"
#include	"fns.h"
#include	"error.h"


#define 	Audio_8Bit_Val		8
#define 	Audio_16Bit_Val		16

#define 	Audio_Mono_Val		1
#define 	Audio_Stereo_Val	2

#define 	Audio_Mic_Val		0
#define 	Audio_Linein_Val	-1

#define		Audio_Speaker_Val	0
#define		Audio_Headphone_Val	-1
#define		Audio_Lineout_Val	-1

#define 	Audio_Pcm_Val		WAVE_FORMAT_PCM
#define 	Audio_Ulaw_Val		(WAVE_FORMAT_PCM+1)
#define 	Audio_Alaw_Val		(WAVE_FORMAT_PCM+2)

#define 	Audio_8K_Val		8000
#define 	Audio_11K_Val		11025
#define 	Audio_22K_Val		22050
#define 	Audio_44K_Val		44100

#define 	Audio_Max_Queue		8

#define BUFLEN		1000

#define NOTINUSE	0x00000000 // header struct is in use
#define INUSE		0x00000001 // header struct is in use
#define INISOPEN	0x00000002 // the microphone is open
#define OUTISOPEN	0x00000004 // the speaker is open
#define MICCLOSEDOWN	0x00000008 // microphone in process of closing down
#define SPEAKCLOSEDOWN	0x00000010 // speaker in process of closing down
#define INPUTISGOING	0x00000020 // microphone is being recorded/read
#define	DATATOREAD	0x00000040 // there is data the user can read
#define OUTSTANDREAD	0x00000080 // buffer sent to microphone and not returned.
#define OUTSTANDWRITE	0x00000100 // buffer sent to speaker and not returned.
#define AUDIOHOLD	0x00000200 // data block waiting to be sent to speaker
#define AUDIOBUSY	0x00000400 // data block sent to speak but not yet written
#define ABORTOUTPUT	0x00000800 // flush pending output before close
#define KICKINPUT	0x00001000 // restart the input stream

#endif
