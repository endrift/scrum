#include "audio.h"

#include <gba_sound.h>

void soundInit(void) {
	REG_SOUNDCNT_X = 0x80;
	REG_SOUNDCNT_L = SND1_R_ENABLE | SND1_L_ENABLE | SND4_R_ENABLE | SND4_L_ENABLE | 0x77;
}

void soundFrame(void) {
}

void playSoundEffect(SoundEffect effect) {
	switch (effect) {
	case SFX_MOVE_UP:
		REG_SOUND1CNT_L = 0x0027;
		REG_SOUND1CNT_H = 0xC1B4;
		REG_SOUND1CNT_X = 0x8500;
		break;
	case SFX_MOVE_DOWN:
		REG_SOUND1CNT_L = 0x0027;
		REG_SOUND1CNT_H = 0xC1B4;
		REG_SOUND1CNT_X = 0x8440;
		break;
	case SFX_DROP:
		REG_SOUND1CNT_L = 0x001F;
		REG_SOUND1CNT_H = 0xE2B4;
		REG_SOUND1CNT_X = 0x8500;
		break;
	case SFX_CLEAR:
		break;
	case SFX_SHOT:
		REG_SOUND1CNT_L = 0x002C;
		REG_SOUND1CNT_H = 0xC340;
		REG_SOUND1CNT_X = 0x8600;
		break;
	case SFX_EXPLOSION:
		REG_SOUND4CNT_L = 0xF400;
		REG_SOUND4CNT_H = 0x8065;
		break;
	}
}