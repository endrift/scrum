#include "intro.h"

#include <gba_compression.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "gameboard.h"
#include "gameParams.h"
#include "rng.h"
#include "text.h"
#include "util.h"

#include "endrift.h"
#include "hud-sprites.h"
#include "tile-palette.h"
#include "tile-data.h"

static void introInit(u32 framecount);
static void introDeinit(void);
static void introFrame(u32 framecount);

static enum {
	LOGO_FADE_IN,
	LOGO_IDLE,
	LOGO_FADE_OUT,
	TITLE_FADE_IN,
	TITLE_FADE_IN_2,
	PRESS_START,
	MODE_SELECT,
	TITLE_FADE_OUT
} state;

Runloop intro = {
	.init = introInit,
	.deinit = introDeinit,
	.frame = introFrame
};

static const GameParameters* modes[] = {
	&easyParams,
	&defaultParams,
	&hardParams,
	0
};

static u32 introStart = 0;
static int modeIndex = 1;

static void switchState(int nextState, u32 framecount) {
	state = nextState;
	introStart = framecount;
	intro.frame(framecount);
}

static void endIntro(u32 framecount) {
	switchState(TITLE_FADE_IN, framecount);
	REG_DISPCNT = 0;
	REG_BLDALPHA = 0x0F00;
	hzero((u16*) VRAM, 48 * 1024);
	hzero(BG_PALETTE, 256);
	REG_BG1CNT = CHAR_BASE(2) | SCREEN_BASE(1) | 1;
	REG_BG3CNT = CHAR_BASE(0) | SCREEN_BASE(2) | 3;
	DMA3COPY(hud_spritesPal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 2));
	BG_COLORS[0] = 0;
	int i;
	for (i = 0; i < 16 * 4; ++i) {
		int color = tile_bluePal[i];
		int r = (color >> 1) & 0xF;
		int g = (color >> 6) & 0xF;
		int b = (color >> 11) & 0xF;
		BG_COLORS[i + 16] = r | g << 5 | b << 10;
	}
	DMA3COPY(tileTiles, TILE_BASE_ADR(0) + 32, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));
	srand(0);
	for (i = 0; i < 32; ++i) {
		int x;
		int width;
		for (x = 0; x < 30;) {
			int seed = rand() >> 16;
			int color = ((seed >> 2) & 3) + 1;
			for (width = (seed & 3) + 1; width; --width, ++x) {
				((u16*) SCREEN_BASE_BLOCK(2))[i * 32 + x] = 1 | CHAR_PALETTE(color);
			}
		}
	}

	mapText(SCREEN_BASE_BLOCK(1), 0, 32, 0, 24, 0);
	renderText("BAD PROGRAMMING METAPHORS", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 9,
		.clipY = 24,
		.clipW = 192,
		.clipH = 16
	}, &largeFont);
	renderText("THE GAME", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 85,
		.clipY = 40,
		.clipW = 192,
		.clipH = 16
	}, &largeFont);
	REG_DISPCNT = MODE_0 | BG1_ON;
}

void introInit(u32 framecount) {
	state = LOGO_FADE_IN;
	introStart = framecount;

	REG_BLDALPHA = 0x0F00; // Whoops, GBA.js doesn't support BLDY in modes 3 - 5
	REG_BLDCNT = 0x3877;
	REG_BG2PA = 0x100;
	REG_BG2PB = 0;
	REG_BG2PC = 0;
	REG_BG2PD = 0x100;
	REG_BG2X = 0;
	REG_BG2Y = 0;

	LZ77UnCompVram((void*) endriftBitmap, (void*) VRAM);
	REG_DISPCNT = MODE_3 | BG2_ON;
	BG_COLORS[0] = 0x7FFF;
}

void introDeinit(void) {
}

void introFrame(u32 framecount) {
	scanKeys();
	u32 keys = keysDown();

	if (keys & KEY_START) {
		if (state < TITLE_FADE_IN) {
			endIntro(framecount);
			return;
		} else if (state < PRESS_START) {
			switchState(PRESS_START, framecount);
			return;
		}
	}

	if (state >= TITLE_FADE_IN_2) {
		u16 y = (framecount >> 2);
		REG_BG3VOFS = y;
		if (!(y & 7)) {
			int x;
			int width;
			for (x = 0; x < 30;) {
				int seed = rand() >> 16;
				int color = ((seed >> 2) & 3) + 1;
				for (width = (seed & 3) + 1; width; --width, ++x) {
					((u16*) SCREEN_BASE_BLOCK(2))[(((y >> 3) + 21) & 31) * 32 + x] = 1 | CHAR_PALETTE(color);
				}
			}
		}
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
			switchState(TITLE_FADE_IN_2, framecount);
		}
		break;
	case TITLE_FADE_IN_2:
		if (framecount - introStart <= 64) {
			REG_DISPCNT = MODE_0 | BG1_ON | BG3_ON;
			REG_BLDCNT = 0x3F78;
			int value = (framecount - introStart) >> 2;
			REG_BLDALPHA = value | (16 - value) << 8;
		} else {
			switchState(PRESS_START, framecount);
		}
		break;
	case PRESS_START:
		if (framecount == introStart) {
			REG_BLDCNT = 0;
			REG_DISPCNT = MODE_0 | BG1_ON | BG3_ON;
			unmapText(SCREEN_BASE_BLOCK(1), 0, 32, 12, 14);
			renderText("PRESS START", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 71,
				.clipY = 96,
				.clipW = 128,
				.clipH = 16
			}, &largeFont);
			mapText(SCREEN_BASE_BLOCK(1), 0, 32, 12, 14, 0);
		}
		if (keys & (KEY_START | KEY_A)) {
			clearBlock(TILE_BASE_ADR(2), 71, 96, 128, 16);
			switchState(MODE_SELECT, framecount);
		}
		break;
	case MODE_SELECT:
		if (framecount == introStart) {
			unmapText(SCREEN_BASE_BLOCK(1), 0, 32, 10, 16);
			renderText("EASY", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 96,
				.clipY = 80,
				.clipW = 128,
				.clipH = 16
			}, &largeFont);
			renderText("NORMAL", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 96,
				.clipY = 96,
				.clipW = 128,
				.clipH = 16
			}, &largeFont);
			renderText("HARD", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 96,
				.clipY = 112,
				.clipW = 128,
				.clipH = 16
			}, &largeFont);
			mapText(SCREEN_BASE_BLOCK(1), 0, 32, 10, 16, 0);
		}
		if (keys & (KEY_START | KEY_A)) {
			currentParams = *modes[modeIndex];
			switchState(TITLE_FADE_OUT, framecount);
		}
		if (keys & KEY_DOWN) {
			++modeIndex;
			if (!modes[modeIndex]) {
				modeIndex = 0;
			}
		}
		if (keys & KEY_UP) {
			--modeIndex;
			if (modeIndex < 0) {
				for (modeIndex = -1; modes[modeIndex + 1]; ++modeIndex);
			}
		}
		break;
	case TITLE_FADE_OUT:
		if (framecount - introStart >= 32) {
			REG_DISPCNT = 0;
			hzero((u16*) VRAM, 48 * 1024);
			hzero(BG_PALETTE, 256);
			setRunloop(&gameBoard);
		} else {
			int value = (framecount - introStart) >> 1;
			REG_BLDCNT = 0x3FFF;
			REG_BLDY = value;
		}
		break;
	}
}