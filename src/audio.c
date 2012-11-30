#include "audio.h"

#include <gba_interrupt.h>
#include <gba_sound.h>
#include <maxmod.h>

#include "soundbank.h"
#include "soundbank_bin.h"

static struct {
	u32 time;
	SoundEffect playing;
} sfx;

void soundInit(void) {
	sfx.time = 0;
	REG_SOUNDCNT_X = 0x80;
	mmInitDefault((mm_addr) soundbank_bin, 8);
	REG_SOUNDCNT_H |= 2;
	REG_SOUNDCNT_L |= SND1_R_ENABLE | SND1_L_ENABLE | SND2_R_ENABLE | SND2_L_ENABLE | SND4_R_ENABLE | SND4_L_ENABLE | 0x77;
	irqSet(IRQ_VBLANK, mmVBlank);
	mmSetVBlankHandler(soundFrame);
}

void soundFrame(void) {
	switch (sfx.playing) {
	case SFX_SELECT:
		if (sfx.time == 0) {
			REG_SOUND2CNT_L = 0xF2C1;
			REG_SOUND2CNT_H = 0x8700;
		} else if (sfx.time & 1) {
			REG_SOUND2CNT_H = 0x0700 - ((sfx.time & 0x3E) << 4);
		} else {
			REG_SOUND2CNT_H = 0x0600 - ((sfx.time & 0x3E) << 3);
		}
		break;
	case SFX_START:
		if (sfx.time == 0) {
			REG_SOUND2CNT_L = 0xF701;
			REG_SOUND2CNT_H = 0x8500;
		} else if (sfx.time & 2) {
			REG_SOUND2CNT_H = 0x0500 + ((sfx.time & 0x6) << 5) + ((sfx.time & 0x78) << 2);
		} else {
			REG_SOUND2CNT_H = 0x0400 + ((sfx.time & 0x6) << 5) + ((sfx.time & 0x78) << 2);
		}
		break;
	default:
		break;
	}
	++sfx.time;
}

void playSoundEffect(SoundEffect effect) {
	sfx.playing = SFX_NONE;
	switch (effect) {
	case SFX_NONE:
		break;
	case SFX_MOVE_UP:
		REG_SOUND1CNT_L = 0x0027;
		REG_SOUND1CNT_H = 0xE1B4;
		REG_SOUND1CNT_X = 0x8500;
		break;
	case SFX_MOVE_DOWN:
		REG_SOUND1CNT_L = 0x0027;
		REG_SOUND1CNT_H = 0xE1B4;
		REG_SOUND1CNT_X = 0x8440;
		break;
	case SFX_DROP:
		REG_SOUND1CNT_L = 0x001F;
		REG_SOUND1CNT_H = 0xF2B4;
		REG_SOUND1CNT_X = 0x8500;
		break;
	case SFX_CLEAR:
		REG_SOUND4CNT_L = 0xF100;
		REG_SOUND4CNT_H = 0x8043;
		break;
	case SFX_SELECT:
	case SFX_START:
		sfx.time = 0;
		sfx.playing = effect;
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
