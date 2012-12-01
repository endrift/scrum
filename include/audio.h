#ifndef AUDIO_H
#define AUDIO_H

#include <maxmod.h>

typedef enum SoundEffect {
	SFX_NONE = 0,
	SFX_MOVE_UP,
	SFX_MOVE_DOWN,
	SFX_DROP,
	SFX_CLEAR,
	SFX_SELECT,
	SFX_START,
	SFX_SHOT,
	SFX_EXPLOSION
} SoundEffect;

void soundInit(void);
void soundFrame(void);

void playModule(mm_word module);
void stopModule(void);
void setModuleVolume(mm_word volume);

void playSoundEffect(SoundEffect effect);

#endif