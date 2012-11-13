#include "intro.h"

#include <gba_compression.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "gameboard.h"
#include "text.h"

#include "endrift.h"
#include "hud-sprites.h"

static void introInit(u32 framecount);
static void introDeinit(void);
static void introFrame(u32 framecount);

static enum {
	LOGO_FADE_IN,
	LOGO_IDLE,
	LOGO_FADE_OUT,
	PRESS_START
} state = LOGO_FADE_IN;

Runloop intro = {
	.init = introInit,
	.deinit = introDeinit,
	.frame = introFrame
};

static u32 introStart = 0;

static void switchState(int nextState, u32 framecount) {
	state = nextState;
	introStart = framecount;
}

static void endIntro(u32 framecount) {
	switchState(PRESS_START, framecount);
	REG_DISPCNT = LCDC_OFF;
	REG_BG1CNT = CHAR_BASE(1) | SCREEN_BASE(0) | 1;
	RegisterRamReset(1 << 3);
	DMA3COPY(hud_spritesPal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 2));
	REG_BLDCNT = 0;
	mapText(SCREEN_BASE_BLOCK(0), 0, 32, 0, 24, 0);
	renderText("THIS GAME NEEDS A NAME", &(Textarea) {
		.destination = TILE_BASE_ADR(1),
		.clipX = 26,
		.clipY = 40,
		.clipW = 192,
		.clipH = 16
	}, &largeFont);

	renderText("PRESS START", &(Textarea) {
		.destination = TILE_BASE_ADR(1),
		.clipX = 74,
		.clipY = 100,
		.clipW = 80,
		.clipH = 16
	}, &largeFont);
	REG_DISPCNT = MODE_0 | BG1_ON;
}

void introInit(u32 framecount) {
	introStart = framecount;

	REG_BLDALPHA = 0x0F00; // Whoops, GBA.js doesn't support BLDY in modes 3 - 5
	REG_BLDCNT = 0x3F7F;

	LZ77UnCompVram((void*) endriftBitmap, (void*) VRAM);
	REG_DISPCNT = MODE_3 | BG2_ON;
}

void introDeinit(void) {
}

void introFrame(u32 framecount) {
	scanKeys();
	u32 keys = keysDown();

	if (keys & KEY_START && state != PRESS_START) {
			endIntro(framecount);
		return;
	}

	switch (state) {
	case LOGO_FADE_IN:
		if (framecount - introStart < 64) {
			REG_BLDALPHA = (framecount - introStart) >> 2 | 0x0F00;
		} else {
			switchState(LOGO_IDLE, framecount);
		}
		break;
	case LOGO_IDLE:
		if (framecount - introStart > 120) {
			BG_COLORS[0] = 0x7FFF;
			switchState(LOGO_FADE_OUT, framecount);
		} else {
			break;
		}
	case LOGO_FADE_OUT:
		REG_BLDALPHA = (framecount - introStart) << 7 | 0x000F;
		if (framecount - introStart > 32) {
			endIntro(framecount);
			switchState(PRESS_START, framecount);
		}
		break;
	case PRESS_START:
		if (keys & KEY_START) {
			RegisterRamReset(1 << 3);
			setRunloop(&gameBoard);
		}
		break;
	}
}