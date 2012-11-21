#include "intro.h"

#include <gba_compression.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "gameboard.h"
#include "gameParams.h"
#include "text.h"
#include "util.h"

#include "endrift.h"
#include "hud-sprites.h"

static void introInit(u32 framecount);
static void introDeinit(void);
static void introFrame(u32 framecount);

static enum {
	LOGO_FADE_IN,
	LOGO_IDLE,
	LOGO_FADE_OUT,
	TITLE_FADE_IN,
	PRESS_START,
	TITLE_FADE_OUT
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
	switchState(TITLE_FADE_IN, framecount);
	REG_DISPCNT = 0;
	REG_BLDALPHA = 0x0F00;
	hzero((u16*) VRAM, 48 * 1024);
	hzero(BG_PALETTE, 256);
	REG_BG1CNT = CHAR_BASE(2) | SCREEN_BASE(1) | 1;
	DMA3COPY(hud_spritesPal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 2));
	BG_COLORS[0] = 0;
	mapText(SCREEN_BASE_BLOCK(1), 0, 32, 0, 24, 0);
	renderText("THIS GAME NEEDS A NAME", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 26,
		.clipY = 40,
		.clipW = 192,
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
	BG_COLORS[0] = 0x7FFF;
}

void introDeinit(void) {
}

void introFrame(u32 framecount) {
	scanKeys();
	u32 keys = keysDown();

	if (keys & KEY_START && state < TITLE_FADE_IN) {
		endIntro(framecount);
		return;
	}

	switch (state) {
	case LOGO_FADE_IN:
		if (framecount - introStart < 64) {
			int value = (framecount - introStart) >> 2;
			REG_BLDALPHA = (value + 1) | (16 - value) << 8;
		} else {
			switchState(LOGO_IDLE, framecount);
			REG_BLDCNT = 0;
		}
		break;
	case LOGO_IDLE:
		if (framecount - introStart > 120) {
			BG_COLORS[0] = 0x0;
			switchState(LOGO_FADE_OUT, framecount);
			REG_BLDCNT = 0x3F7F;
		} else {
			break;
		}
	case LOGO_FADE_OUT:
		if (framecount - introStart >= 32) {
			endIntro(framecount);
		} else {
			int value = (framecount - introStart) >> 1;
			REG_BLDALPHA = (16 - value) | value << 8;
		}
		break;
	case TITLE_FADE_IN:
		if (framecount - introStart <= 64) {
			int value = (framecount - introStart) >> 2;
			REG_BLDALPHA = value | (16 - value) << 8;
		} else {
			renderText("PRESS START", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 74,
				.clipY = 100,
				.clipW = 80,
				.clipH = 16
			}, &largeFont);
			REG_BLDCNT = 0;
			switchState(PRESS_START, framecount);
		}
		break;
	case PRESS_START:
		if (keys & KEY_START) {
			switchState(TITLE_FADE_OUT, framecount);
			REG_BLDCNT = 0x3F7F;
		}
		break;
	case TITLE_FADE_OUT:
		if (framecount - introStart >= 32) {
			currentParams = defaultParams;
			REG_DISPCNT = 0;
			hzero((u16*) VRAM, 48 * 1024);
			hzero(BG_PALETTE, 256);
			setRunloop(&gameBoard);
		} else {
			int value = (framecount - introStart) >> 1;
			REG_BLDALPHA = (16 - value) | value << 8;
		}
		break;
	}
}