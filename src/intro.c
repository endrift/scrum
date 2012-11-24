#include "intro.h"

#include <gba_compression.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "gameboard.h"
#include "gameParams.h"
#include "m7.h"
#include "rng.h"
#include "text.h"
#include "util.h"

#include "cursor.h"
#include "endrift.h"
#include "hud-sprites.h"
#include "tile-palette.h"
#include "tile-large.h"
#include "title.h"

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
static Sprite cursor = {
	.x = 80,
	.size = 1
};

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
	REG_BG2CNT = CHAR_BASE(0) | SCREEN_BASE(2) | BG_SIZE_2 | BG_WRAP | 3;
	DMA3COPY(hud_spritesPal, &BG_COLORS[16 * 4], DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 2));
	DMA3COPY(hud_spritesPal, &OBJ_COLORS[0], DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 2));
	DMA3COPY(titlePal, &BG_COLORS[16 * 5], DMA16 | DMA_IMMEDIATE | (titlePalLen >> 1));
	DMA3COPY(tile_bluePal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (tile_bluePalLen << 1));
	BG_COLORS[0] = 0;
	DMA3COPY(tile_largeTiles, TILE_BASE_ADR(0) + 64, DMA16 | DMA_IMMEDIATE | (tile_largeTilesLen >> 1));
	DMA3COPY(cursorTiles, OBJ_BASE_ADR, DMA16 | DMA_IMMEDIATE | (cursorTilesLen >> 1));
	srand(0);
	int i;
	for (i = 0; i < 64; ++i) {
		int x;
		int width;
		for (x = 0; x < 32;) {
			int seed = rand() >> 16;
			int color = (seed >> 2) & 3;
			for (width = (seed & 3) + 1; width; --width, ++x) {
				if (x > 32) {
					break;
				}
				((u16*) SCREEN_BASE_BLOCK(2))[i * 64 + x] = 0x201 + color * 0x404;
				((u16*) SCREEN_BASE_BLOCK(2))[(2 * i + 1) * 32 + x] = 0x403 + color * 0x404;
			}
		}
	}
	DMA3COPY(titleTiles, TILE_BASE_ADR(2), DMA16 | DMA_IMMEDIATE | (titleTilesLen >> 1));
	mapText(SCREEN_BASE_BLOCK(1), 0, 32, 0, 11, 5);

	m7Context.y = 128 << 8;
	m7Context.x = 256 << 8;
	m7Context.d = 250;
	m7Context.w = 120;
	for (i = 0; i < 160; ++i) {
		if (i >= 64) {
			m7Context.bgFade[i] = 0;
		} else {
			m7Context.bgFade[i] = (64 - i) >> 2;
		}
		m7Context.div16[i] = ((1 << 24) / (i + 100)) >> 8;
	}
	enableMode7(1);

	REG_DISPCNT = MODE_1 | BG1_ON;
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

	clearSpriteTable();
	writeSpriteTable();
}

void introDeinit(void) {
	enableMode7(0);
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
		m7Context.z = (y & 0x3FF) << 8;
		if (!(framecount & 0x3F)) {
			int x;
			int width;
			for (x = 0; x < 32;) {
				int seed = rand() >> 16;
				int color = (seed >> 2) & 3;
				for (width = (seed & 3) + 1; width; --width, ++x) {
					if (x > 32) {
						break;
					}
					((u16*) SCREEN_BASE_BLOCK(2))[(((y >> 3) + 22) & 63) * 32 + x] = 0x201 + color * 0x404;
					((u16*) SCREEN_BASE_BLOCK(2))[(((y >> 3) + 23) & 63) * 32 + x] = 0x403 + color * 0x404;
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
		if (framecount - introStart < 32) {
			REG_DISPCNT = MODE_1 | BG1_ON | BG2_ON;
			REG_BLDCNT = 0x00C4;
			int value = (framecount - introStart) >> 1;
			m7Context.fadeOffset = 17 - value;
		} else {
			switchState(PRESS_START, framecount);
		}
		break;
	case PRESS_START:
		if (framecount == introStart) {
			m7Context.fadeOffset = 1;
			REG_BLDCNT = 0x00C4;
			REG_DISPCNT = MODE_1 | BG1_ON | BG2_ON | OBJ_ON | OBJ_1D_MAP;
			unmapText(SCREEN_BASE_BLOCK(1), 0, 32, 12, 14);
			renderText("PRESS START", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 71,
				.clipY = 96,
				.clipW = 128,
				.clipH = 16
			}, &largeFont);
			mapText(SCREEN_BASE_BLOCK(1), 0, 32, 12, 14, 4);
		}
		if (keys & (KEY_START | KEY_A)) {
			clearBlock(TILE_BASE_ADR(2), 71, 96, 128, 16);
			switchState(MODE_SELECT, framecount);
		}
		break;
	case MODE_SELECT:
		if (framecount == introStart) {
			unmapText(SCREEN_BASE_BLOCK(1), 0, 32, 12, 18);
			renderText("EASY", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 96,
				.clipY = 96,
				.clipW = 128,
				.clipH = 16
			}, &largeFont);
			renderText("NORMAL", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 96,
				.clipY = 112,
				.clipW = 128,
				.clipH = 16
			}, &largeFont);
			renderText("HARD", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 96,
				.clipY = 128,
				.clipW = 128,
				.clipH = 16
			}, &largeFont);
			mapText(SCREEN_BASE_BLOCK(1), 0, 32, 12, 18, 4);

			cursor.y = 112;
			appendSprite(&cursor);
			writeSpriteTable();
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
			cursor.y = 96 + 16 * modeIndex;
			updateSprite(&cursor, 0);
		}
		if (keys & KEY_UP) {
			--modeIndex;
			if (modeIndex < 0) {
				for (modeIndex = -1; modes[modeIndex + 1]; ++modeIndex);
			}
			cursor.y = 96 + 16 * modeIndex;
			updateSprite(&cursor, 0);
		}
		writeSpriteTable();
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
			m7Context.fadeOffset = value + 1;
		}
		break;
	}
}