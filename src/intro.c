#include "intro.h"

#include <gba_compression.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_sprites.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "gameboard.h"
#include "gameParams.h"
#include "highscore.h"
#include "m7.h"
#include "rng.h"
#include "save.h"
#include "text.h"
#include "util.h"

#include "audio.h"
#include "cursor.h"
#include "cycle.h"
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

static u32 introStart = 0;
static int modeIndex = 1;
static Sprite cursor = {
	.x = 56,
	.size = 1
};

static AffineSprite cycle = {
	.sprite = {
		.x = 88,
		.y = 4,
		.size = 3,
		.transformed = 1,
		.base = 68,
		.priority = 2,
		.mode = 1
	},
	.affine = {
		.sX = 0x100,
		.sY = -0x100
	}
};

static Sprite circle = {
	.x = 88,
	.y = 4,
	.size = 3,
	.base = 4,
	.priority = 2,
	.mode = 1
};

static Runloop* destinationMode;

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
	DMA3COPY(cycleTiles, OBJ_BASE_ADR + 0x80, DMA16 | DMA_IMMEDIATE | (cycleTilesLen >> 1));
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
	renderText("PRESS START", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 71,
		.clipY = 96,
		.clipW = 128,
		.clipH = 16
	}, &largeFont);
	renderText("NEW GAME", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 71,
		.clipY = 112,
		.clipW = 128,
		.clipH = 16
	}, &largeFont);
	renderText("OPTIONS", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 71,
		.clipY = 128,
		.clipW = 128,
		.clipH = 16
	}, &largeFont);
	renderText("STASH POP", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 71,
		.clipY = 144,
		.clipW = 128,
		.clipH = 16
	}, &largeFont);
	renderText(modes[0]->modeName, &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 72,
		.clipY = 160,
		.clipW = 128,
		.clipH = 16
	}, &largeFont);
	renderText(modes[1]->modeName, &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 72,
		.clipY = 176,
		.clipW = 128,
		.clipH = 16
	}, &largeFont);
	renderText(modes[2]->modeName, &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 72,
		.clipY = 192,
		.clipW = 128,
		.clipH = 16
	}, &largeFont);

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
	cycle.sprite.transformGroup = 0;
	cycle.id = appendSprite(&cycle.sprite);
	appendSprite(&circle);
	ObjAffineSet(&cycle.affine, affineTable(0), 1, 8);
	writeSpriteTable();

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

	initSRAM();
}

void introDeinit(void) {
	enableMode7(0);
	REG_BLDCNT = 0x0000;
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
		m7Context.z = framecount << 7;
		if (!(framecount & 0x1F)) {
			int x;
			int width;
			for (x = 0; x < 32;) {
				int seed = rand() >> 16;
				int color = (seed >> 2) & 3;
				for (width = (seed & 3) + 1; width; --width, ++x) {
					if (x > 32) {
						break;
					}
					((u16*) SCREEN_BASE_BLOCK(2))[(((framecount >> 4) + 22) & 63) * 32 + x] = 0x201 + color * 0x404;
					((u16*) SCREEN_BASE_BLOCK(2))[(((framecount >> 4) + 23) & 63) * 32 + x] = 0x403 + color * 0x404;
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
		if (framecount - introStart <= 32) {
			REG_DISPCNT = MODE_1 | BG1_ON | BG2_ON | OBJ_ON | OBJ_1D_MAP;
			REG_BLDCNT = 0x24C4;
			int value = (framecount - introStart) >> 1;
			REG_BLDALPHA = (value >> 1) | ((value << 7) & 0x1F00);
			m7Context.fadeOffset = 17 - value;
		} else {
			switchState(PRESS_START, framecount);
		}
		break;
	case PRESS_START:
		if (framecount == introStart) {
			m7Context.fadeOffset = 1;
			REG_BLDCNT = 0x24C4;
			REG_BLDALPHA = 0x0808;
			REG_DISPCNT = MODE_1 | BG1_ON | BG2_ON | OBJ_ON | OBJ_1D_MAP;
			unmapText(SCREEN_BASE_BLOCK(1), 0, 32, 12, 14);
			mapText(SCREEN_BASE_BLOCK(1), 0, 32, 12, 14, 4);
		}
		if (keys & (KEY_START | KEY_A)) {
			playSoundEffect(SFX_SELECT);
			switchState(MODE_SELECT, framecount);
		}
		if (keys & KEY_SELECT) {
			if (isSavedGame()) {
				loadGame();
				destinationMode = &gameBoard;
				playSoundEffect(SFX_START);
				switchState(TITLE_FADE_OUT, framecount);
			}
		}
		break;
	case MODE_SELECT:
		if (framecount == introStart) {
			unmapText(SCREEN_BASE_BLOCK(1), 0, 32, 12, 18);
			remapText(SCREEN_BASE_BLOCK(1), 0, 20, 0, 32, 12, 18, 4);

			cursor.y = 96 + 16 * modeIndex;
			appendSprite(&cursor);
		}
		if (keys & (KEY_START | KEY_A)) {
			currentParams = *modes[modeIndex];
			destinationMode = &gameBoard;
			playSoundEffect(SFX_START);
			switchState(TITLE_FADE_OUT, framecount);
		}
		if (keys & (KEY_SELECT)) {
			currentParams = *modes[modeIndex];
			destinationMode = &displayHighScores;
			switchState(TITLE_FADE_OUT, framecount);
		}
		if (keys & KEY_DOWN) {
			++modeIndex;
			playSoundEffect(SFX_MOVE_DOWN);
			if (!modes[modeIndex]) {
				modeIndex = 0;
			}
			cursor.y = 96 + 16 * modeIndex;
			updateSprite(&cursor, 2);
		}
		if (keys & KEY_UP) {
			--modeIndex;
			playSoundEffect(SFX_MOVE_UP);
			if (modeIndex < 0) {
				for (modeIndex = -1; modes[modeIndex + 1]; ++modeIndex);
			}
			cursor.y = 96 + 16 * modeIndex;
			updateSprite(&cursor, 2);
		}
		break;
	case TITLE_FADE_OUT:
		if (framecount - introStart >= 32) {
			REG_DISPCNT = 0;
			hzero((u16*) VRAM, 48 * 1024);
			hzero(BG_PALETTE, 256);
			gameMode = modeIndex;
			setRunloop(destinationMode);
		} else {
			int value = (framecount - introStart) >> 1;
			REG_BLDCNT = 0x3FFF;
			REG_BLDALPHA = ((16 - value) >> 1) | (((16 - value) << 6) & 0x1F00);
			m7Context.fadeOffset = value + 1;
		}
		break;
	}

	if (state >= TITLE_FADE_IN) {
		cycle.affine.theta -= 128;
		ObjAffineSet(&cycle.affine, affineTable(0), 1, 8);
		writeSpriteTable();
	}
}