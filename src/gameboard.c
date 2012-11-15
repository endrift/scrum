#include "gameboard.h"

#include <gba_dma.h>
#include <gba_input.h>
#include <gba_sound.h>
#include <gba_sprites.h>
#include <gba_video.h>

#include "rng.h"
#include "sprite.h"
#include "key.h"
#include "minigame.h"
#include "text.h"
#include "util.h"

#include "tile-palette.h"
#include "tile-data.h"
#include "game-backdrop.h"
#include "hud-sprites.h"

Runloop gameBoard = {
	.init = gameBoardInit,
	.deinit = gameBoardDeinit,
	.frame = gameBoardFrame
};

typedef struct Row {
	u8 color[GAMEBOARD_COLS + GAMEBOARD_DEADZONE];
	u8 width;
} Row;

typedef struct Block {
	Sprite spriteL;
	Sprite spriteR;
	int indexL;
	int indexR;
	int width;
	int color;
} Block;

typedef struct GameBoard {
	Row rows[GAMEBOARD_ROWS];

	Block active;
	Block next;
	int activeY;
	int timer;

	int score;
	int lines;
	int bugs;
} GameBoard;

static GameBoard board;

const static Sprite bugSprite = {
	.x = 183,
	.y = 119,
	.base = 186,
	.shape = 1,
	.size = 2,
	.palette = 4
};

u16 timerPalette[16];

static void repeatHandler(KeyContext* context, int key);
static KeyContext keyContext = {
	.next = {},
	.active = 0,
	.startDelay = 12,
	.repeatDelay = 8,
	.repeatHandler = repeatHandler
};

static void drawBoard(void) {
	u16* mapData = SCREEN_BASE_BLOCK(1);

	int x, y;
	for (y = 0; y < GAMEBOARD_ROWS; ++y) {
		for (x = 0; x < board.rows[y].width; ++x) {
			mapData[x + y * 32] = 1 | CHAR_PALETTE(board.rows[y].color[x]);
		}
		for (; x < GAMEBOARD_COLS + GAMEBOARD_DEADZONE; ++x) {
			mapData[x + y * 32] = 0;
		}
	}
}

static void resetPlayfield(void) {
	int x, y;
	for (y = 0; y < GAMEBOARD_ROWS; ++y) {
		for (x = 0; x < GAMEBOARD_COLS + GAMEBOARD_DEADZONE; ++x) {
			((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32 + 97] = 5 | CHAR_PALETTE(4);
		}
		((u16*) SCREEN_BASE_BLOCK(2))[y * 32 + 97 + 16] = 6 | CHAR_PALETTE(4);
	}
}

static void resetBackdrop(void) {
	int x, y;
	for (y = 0; y < 20; ++y) {
		for (x = 0; x < 30; ++x) {
			((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32] = (((x + y) % 3) + 2) | CHAR_PALETTE(4);
		}
	}

	resetPlayfield();

	for (y = 0; y < 16; ++y) {
		for (x = 0; x < 6; ++x) {
			if ((y & 3) == 3 && y != 15) {
				((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32 + 119] = 9 | CHAR_PALETTE(4);
			} else if (y & 1 && y != 15) {
				((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32 + 119] = 8 | CHAR_PALETTE(4);
			} else {
				((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32 + 119] = 7 | CHAR_PALETTE(4);
			}
		}
	}

	appendSprite(&(Sprite) {
		.x = 0,
		.y = 16,
		.base = 248,
		.palette = 4,
		.size = 2
	});

	appendSprite(&(Sprite) {
		.x = 144,
		.y = 16,
		.base = 248,
		.palette = 5,
		.size = 2,
		.hflip = 1
	});

	appendSprite(&(Sprite) {
		.x = 0,
		.y = 104,
		.base = 280,
		.palette = 4,
		.shape = 2,
		.size = 3
	});

	appendSprite(&(Sprite) {
		.x = 144,
		.y = 104,
		.base = 280,
		.palette = 5,
		.shape = 2,
		.size = 3,
		.hflip = 1
	});


	for (x = 0; x < 7; ++x) {
		appendSprite(&(Sprite) {
			.x = 16 * x + 24,
			.y = 16,
			.base = 249,
			.palette = 4,
			.shape = 1,
			.size = 0
		});

		appendSprite(&(Sprite) {
			.x = 16 * x + 24,
			.y = 152,
			.base = 473,
			.palette = 4,
			.shape = 1,
			.size = 0
		});
	}


	appendSprite(&(Sprite) {
		.x = 136,
		.y = 16,
		.base = 249,
		.palette = 5,
	});

	appendSprite(&(Sprite) {
		.x = 136,
		.y = 152,
		.base = 473,
		.palette = 5,
	});

	appendSprite(&(Sprite) {
		.x = 176,
		.y = 16,
		.base = 248,
		.palette = 4,
		.size = 2
	});

	appendSprite(&(Sprite) {
		.x = 208,
		.y = 16,
		.base = 248,
		.palette = 4,
		.size = 2,
		.hflip = 1
	});

	appendSprite(&(Sprite) {
		.x = 176,
		.y = 104,
		.base = 280,
		.palette = 4,
		.shape = 2,
		.size = 3
	});

	appendSprite(&(Sprite) {
		.x = 208,
		.y = 104,
		.base = 280,
		.palette = 4,
		.shape = 2,
		.size = 3,
		.hflip = 1
	});

	for (y = 0; y < 3; ++y) {
		appendSprite(&(Sprite) {
			.x = 0,
			.y = 16 * y + 48,
			.base = 280,
			.palette = 4,
			.shape = 0,
			.size = 2
		});

		appendSprite(&(Sprite) {
			.x = 144,
			.y = 16 * y + 48,
			.base = 280,
			.palette = 5,
			.shape = 0,
			.size = 2,
			.hflip = 1
		});

		appendSprite(&(Sprite) {
			.x = 176,
			.y = 16 * y + 48,
			.base = 280,
			.palette = 4,
			.shape = 0,
			.size = 2
		});

		appendSprite(&(Sprite) {
			.x = 208,
			.y = 16 * y + 48,
			.base = 280,
			.palette = 4,
			.shape = 0,
			.size = 2,
			.hflip = 1
		});
	}
}

static void layBlock(void);

static void updateScore(void) {
	static char buffer[10] = "000000000\0";

	// TODO: Unhard-code these coordinates?
	if (board.score > 99999) {
		formatNumber(buffer, 9, board.score);
		renderText(buffer, &(Textarea) {
			.destination = TILE_BASE_ADR(2),
			.clipX = 185,
			.clipY = 40,
			.clipW = 64,
			.clipH = 16,
			.baseline = 0
		}, &thinFont);
	} else {
		formatNumber(&buffer[4], 5, board.score);
		renderText(&buffer[4], &(Textarea) {
			.destination = TILE_BASE_ADR(2),
			.clipX = 185,
			.clipY = 40,
			.clipW = 64,
			.clipH = 16,
			.baseline = 0
		}, &largeFont);
	}

	formatNumber(&buffer[4], 5, board.lines);
	renderText(&buffer[4], &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 185,
		.clipY = 72,
		.clipW = 64,
		.clipH = 16,
		.baseline = 0
	}, &largeFont);

	formatNumber(&buffer[6], 3, board.bugs);
	renderText(&buffer[6], &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 203,
		.clipY = 120,
		.clipW = 32,
		.clipH = 16,
		.baseline = 0
	}, &largeFont);
}

static void updateBlockSprite(Block* block) {
	block->spriteL.palette = block->color;
	block->spriteR.palette = block->color;

	switch (block->width) {
	case 1:
		block->spriteL.shape = 0;
		block->spriteR.disable = 1;
		break;
	case 2:
		block->spriteL.shape = 1;
		block->spriteR.disable = 1;
		break;
	case 3:
		block->spriteL.shape = 1;
		block->spriteR.disable = 0;
		block->spriteR.shape = 0;
		break;
	case 4:
		block->spriteL.shape = 1;
		block->spriteR.disable = 0;
		block->spriteR.shape = 1;
		break;
	}
}

static void genBlock(void) {
	u32 seed = rand() >> 16;
	board.active = board.next;
	board.active.spriteL.mode = 1;
	board.active.spriteL.x = 0x88;
	board.active.spriteR.mode = 1;
	board.active.spriteR.x = 0x98;
	board.next.width = (seed & 3) + 1;
	board.next.color = (seed >> 2) & 3;;

	updateBlockSprite(&board.next);
}

static void genRow(int row) {
	int i;
	int y = board.activeY;
	board.activeY = row;
	board.rows[row].width = 0;
	for (i = 0; i < 4; ++i) {
		u32 seed = rand() >> 16;
		int width = (seed % 3) + 1;
		int color = (seed >> 2) & 3;
		int x;
		for (x = 0; x < width; ++x) {
			board.rows[row].color[x + board.rows[row].width] = color;
		}
		board.rows[row].width += width;
	}
	board.activeY = y;
}

static void resetBoard(void) {
	genBlock();
	genBlock(); // Ensure that a block is actually queued
	int y;
	for (y = 0; y < GAMEBOARD_ROWS; ++y) {
		board.rows[y].width = 0;
		genRow(y);
	}
}

static void removeRow(void) {
	int x;
	Row* row = &board.rows[board.activeY];
	int color = row->color[row->width - 1];
	int score = -board.active.width;
	if (row->width > GAMEBOARD_COLS) {
		board.bugs += row->width - GAMEBOARD_COLS;
	}
	for (x = row->width - 1; x >= 0; --x) {
		if (row->color[x] != color) {
			break;
		}
		--row->width;
		++score;
	}
	if (x < 0) {
		++board.lines;
		score *= 2;
		int y;
		for (y = board.activeY; y < GAMEBOARD_ROWS - 1; ++y) {
			board.rows[y] = board.rows[y + 1];
		}
		genRow(GAMEBOARD_ROWS - 1);
	}
	board.score += score;
	REG_SOUND4CNT_L = 0xF200;
	REG_SOUND4CNT_H = 0x8062;
}

static void layBlock(void) {
	int nextWidth = board.rows[board.activeY].width + board.active.width;
	int i;
	for (i = board.rows[board.activeY].width; i < nextWidth; ++i) {
		board.rows[board.activeY].color[i] = board.active.color;
	}
	board.rows[board.activeY].width = i;
}

static void dropBlock(void) {
	REG_SOUND1CNT_L = 0x001F;
	REG_SOUND1CNT_H = 0xE2B4;
	REG_SOUND1CNT_X = 0x8500;
	layBlock();
	if (board.rows[board.activeY].width >= GAMEBOARD_COLS) {
		removeRow();
		updateScore();
	}
	genBlock();

	board.timer = 0;
}

static void updateTimer() {
	++board.timer;
	int i;
	for (i = 0; i < 16; ++i) {
		OBJ_COLORS[16 * 5 + i] = timerPalette[i] + ((board.timer >> 4) & 0xF);
	}
	if (!((board.timer + 1) & 0xFF)) {
		dropBlock();
	}
}

static void blockUp(void) {
	--board.activeY;
	if (board.activeY < 0) {
		board.activeY = 0;
	} else {
		REG_SOUND1CNT_L = 0x0027;
		REG_SOUND1CNT_H = 0xA1B4;
		REG_SOUND1CNT_X = 0x8500;
	}
}

static void blockDown(void) {
	++board.activeY;
	if (board.activeY >= GAMEBOARD_ROWS) {
		board.activeY = GAMEBOARD_ROWS - 1;
	} else {
		REG_SOUND1CNT_L = 0x0027;
		REG_SOUND1CNT_H = 0xA1B4;
		REG_SOUND1CNT_X = 0x8440;
	}
}

static void hideBoard(void) {
	board.active.spriteL.disable = 1;
	board.active.spriteR.disable = 1;
	board.next.spriteL.disable = 1;
	board.next.spriteR.disable = 1;
	updateSprite(&board.active.spriteL, 0);
	updateSprite(&board.active.spriteR, 1);
	updateSprite(&board.next.spriteL, 2);
	updateSprite(&board.next.spriteR, 3);
	writeSpriteTable();
}

static void repeatHandler(KeyContext* context, int keys) {
	(void) (context);
	if (keys & KEY_UP) {
		blockUp();
	}
	if (keys & KEY_DOWN) {
		blockDown();
	}
}

void gameBoardInit(u32 framecount) {
	// TODO: store this in RAM so we don't have to copy it out of the cart each time
	DMA3COPY(tileTiles, TILE_BASE_ADR(0) + 32, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));
	DMA3COPY(tileTiles, OBJ_BASE_ADR, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));
	DMA3COPY(tileTiles, OBJ_BASE_ADR + 32, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));

	DMA3COPY(game_backdropTiles, TILE_BASE_ADR(0) + 64, DMA16 | DMA_IMMEDIATE | (game_backdropTilesLen >> 1));

	DMA3COPY(hud_spritesTiles, TILE_BASE_ADR(4) + 0x1400, DMA16 | DMA_IMMEDIATE | (hud_spritesTilesLen >> 1));

	clearSpriteTable();
	board.next.spriteL.raw.a = 0x408C;
	board.next.spriteR.raw.a = 0x408C;
	insertSprite(&board.next.spriteL, 0);
	insertSprite(&board.next.spriteR, 1);
	insertSprite(&board.next.spriteL, 2);
	insertSprite(&board.next.spriteR, 3);
	appendSprite(&bugSprite);
	writeSpriteTable();

	resetBackdrop();

	mapText(SCREEN_BASE_BLOCK(3), 20, 32, 0, 17, 5);

	// TODO: Move to constants
	renderText("SCORE", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 186,
		.clipY = 24,
		.clipW = 64,
		.clipH = 16,
		.baseline = 0
	}, &largeFont);

	renderText("LINES", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 192,
		.clipY = 56,
		.clipW = 64,
		.clipH = 16,
		.baseline = 0
	}, &largeFont);

	renderText("FUNC", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 195,
		.clipY = 88,
		.clipW = 64,
		.clipH = 16,
		.baseline = 0
	}, &largeFont);

	renderText("DEBUG", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 186,
		.clipY = 136,
		.clipW = 64,
		.clipH = 16,
		.baseline = 0
	}, &largeFont);

	srand(framecount);
	updateScore();
	gameBoardSetup();
	resetBoard();

	gameBoardFrame(framecount);

	DMA3COPY(tile_bluePal, &OBJ_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 4));
	DMA3COPY(game_backdropPal, &BG_COLORS[16 * 4], DMA16 | DMA_IMMEDIATE | (game_backdropPalLen >> 1));
	DMA3COPY(hud_spritesPal, &BG_COLORS[16 * 5], DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 2));
	DMA3COPY(hud_spritesPal, &OBJ_COLORS[16 * 4], DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 1));
	DMA3COPY(hud_spritesPal, timerPalette, DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 1));
}

void gameBoardDeinit() {
}

void gameBoardFrame(u32 framecount) {
	// Draw the last frame so we can take forever on the next
	drawBoard();
	updateSprite(&board.active.spriteL, 0);
	updateSprite(&board.active.spriteR, 1);
	updateSprite(&board.next.spriteL, 2);
	updateSprite(&board.next.spriteR, 3);
	writeSpriteTable();

	scanKeys();
	u16 keys = keysDown();
	u16 unkeys = keysUp();

	if (unkeys) {
		stopRepeat(&keyContext, unkeys);
	}

	if (keys & KEY_UP) {
		startRepeat(&keyContext, framecount, KEY_UP);
		blockUp();
	}

	if (keys & KEY_DOWN) {
		startRepeat(&keyContext, framecount, KEY_DOWN);
		blockDown();
	}

	doRepeat(&keyContext, framecount);

	if (keys & KEY_A && board.timer > 4) {
		dropBlock();
	}

	updateTimer();

	REG_BLDALPHA = 0x0F0B;
	board.active.spriteL.y = board.active.spriteR.y = 160 - 8 - 8 * GAMEBOARD_ROWS + (board.activeY << 3);
	board.next.spriteL.x = 0xC4 + (3 - board.next.width) * 4;
	board.next.spriteR.x = 0xD4 + (3 - board.next.width) * 4;

	if (keys & KEY_B) {
		gameBoard.frame = minigameFrame;
		hideBoard();
		minigameInit();
	}
}

void gameBoardSetup(void) {
	DMA3COPY(tile_bluePal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 4));
	resetPlayfield();

	REG_BG0CNT = CHAR_BASE(0) | SCREEN_BASE(2) | 3;
	REG_BG1CNT = CHAR_BASE(2) | SCREEN_BASE(3) | 1;
	REG_BG2CNT = CHAR_BASE(0) | SCREEN_BASE(1) | 2;
	REG_BG2HOFS = -8;
	REG_BG2VOFS = -24;
	REG_DISPCNT = MODE_0 | BG0_ON | BG1_ON | BG2_ON | OBJ_ON | WIN0_ON;
	REG_BLDCNT = 0x0400;

	REG_WIN0H = 0x08A8;
	REG_WIN0V = 0x1898;
	REG_WININ = 0x3B3F;
	REG_WINOUT = 0x001B;

	board.active.spriteL.disable = 0;
	board.next.spriteL.disable = 0;
	updateBlockSprite(&board.active);
	updateBlockSprite(&board.next);
	writeSpriteTable();

	REG_SOUNDCNT_X = 0x80;
	REG_SOUNDCNT_L = SND1_R_ENABLE | SND1_L_ENABLE | SND4_R_ENABLE | SND4_L_ENABLE | 0x33;
}