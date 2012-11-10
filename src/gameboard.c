#include "gameboard.h"

#include <gba_dma.h>
#include <gba_input.h>
#include <gba_sprites.h>
#include <gba_video.h>

#include "rng.h"
#include "sprite.h"

#include "tile-palette.h"
#include "tile-data.h"
#include "game-backdrop.h"
#include "frame.h"

Runloop gameBoard = {
	.init = gameBoardInit,
	.deinit = gameBoardDeinit,
	.frame = gameBoardFrame
};

typedef struct GameBoard {
	struct Row {
		u8 color[GAMEBOARD_COLS + GAMEBOARD_DEADZONE];
		u8 width;
	} rows[GAMEBOARD_ROWS];

	Sprite activeBlockL;
	Sprite activeBlockR;
	int activeY;
	int activeWidth;
	int activeColor;

	int score;
	int lines;
	int bugs;
} GameBoard;

static GameBoard board;

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

static void resetBackdrop(void) {
	int x, y;
	for (y = 0; y < 20; ++y) {
		for (x = 0; x < 30; ++x) {
			((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32] = (((x + y) % 3) + 2) | CHAR_PALETTE(4);
		}
	}

	for (y = 0; y < GAMEBOARD_ROWS; ++y) {
		for (x = 0; x < GAMEBOARD_COLS + GAMEBOARD_DEADZONE; ++x) {
			((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32 + 97] = 5 | CHAR_PALETTE(4);
		}
		((u16*) SCREEN_BASE_BLOCK(2))[y * 32 + 97 + 16] = 6 | CHAR_PALETTE(4);
	}

	appendSprite(&(Sprite) {
		.x = 0,
		.y = 16,
		.base = 128,
		.palette = 4,
		.size = 2
	});

	appendSprite(&(Sprite) {
		.x = 144,
		.y = 16,
		.base = 128,
		.palette = 4,
		.size = 2,
		.hflip = 1
	});

	appendSprite(&(Sprite) {
		.x = 0,
		.y = 104,
		.base = 132,
		.palette = 4,
		.shape = 2,
		.size = 3
	});

	appendSprite(&(Sprite) {
		.x = 144,
		.y = 104,
		.base = 132,
		.palette = 4,
		.shape = 2,
		.size = 3,
		.hflip = 1
	});


	for (x = 0; x < 7; ++x) {
		appendSprite(&(Sprite) {
			.x = 16 * x + 32,
			.y = 16,
			.base = 129,
			.palette = 4,
			.shape = 1,
			.size = 0
		});

		appendSprite(&(Sprite) {
			.x = 16 * x + 32,
			.y = 152,
			.base = 158,
			.palette = 4,
			.shape = 1,
			.size = 0
		});
	}

	for (y = 0; y < 3; ++y) {
		appendSprite(&(Sprite) {
			.x = 0,
			.y = 16 * y + 48,
			.base = 132,
			.palette = 4,
			.shape = 0,
			.size = 2
		});

		appendSprite(&(Sprite) {
			.x = 144,
			.y = 16 * y + 48,
			.base = 132,
			.palette = 4,
			.shape = 0,
			.size = 2,
			.hflip = 1
		});
	}
}

static void genBlock(void) {
	u32 seed = rand() >> 16;
	board.activeWidth = (seed & 3) + 1;
	board.activeColor = (seed >> 2) & 3;
	board.activeBlockL.palette = board.activeColor;
	board.activeBlockR.palette = board.activeColor;

	switch (board.activeWidth) {
	case 1:
		board.activeBlockL.shape = 0;
		board.activeBlockR.disable = 1;
		break;
	case 2:
		board.activeBlockL.shape = 1;
		board.activeBlockR.disable = 1;
		break;
	case 3:
		board.activeBlockL.shape = 1;
		board.activeBlockR.disable = 0;
		board.activeBlockR.shape = 0;
		break;
	case 4:
		board.activeBlockL.shape = 1;
		board.activeBlockR.disable = 0;
		board.activeBlockR.shape = 1;
		break;
	}
}

static void resetBoard(void) {
	int y;
	for (y = 0; y < GAMEBOARD_ROWS; ++y) {
		board.rows[y].width = 0;
	}
	srand(42);
	genBlock();
}

static void removeRow(void) {
	++board.lines;
	if (board.rows[board.activeY].width > GAMEBOARD_COLS) {
		board.bugs += board.rows[board.activeY].width;
	}
	int y;
	for (y = board.activeY; y < GAMEBOARD_ROWS - 1; ++y) {
		board.rows[y] = board.rows[y + 1];
	}
	board.rows[GAMEBOARD_ROWS - 1].width = 0;
}

static void layBlock(void) {
	int nextWidth = board.rows[board.activeY].width + board.activeWidth;
	int i;
	for (i = board.rows[board.activeY].width; i < nextWidth; ++i) {
		board.rows[board.activeY].color[i] = board.activeColor;
	}
	board.rows[board.activeY].width = i;
	if (nextWidth >= GAMEBOARD_COLS) {
		removeRow();
	}
	genBlock();
}

void gameBoardInit() {
	DMA3COPY(tile_bluePal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 4));
	DMA3COPY(tile_bluePal, &OBJ_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 4));
	// TODO: store this in RAM so we don't have to copy it out of the cart each time
	DMA3COPY(tileTiles, TILE_BASE_ADR(0) + 32, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));
	DMA3COPY(tileTiles, OBJ_BASE_ADR, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));
	DMA3COPY(tileTiles, OBJ_BASE_ADR + 32, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));
	DMA3COPY(tileTiles, OBJ_BASE_ADR + 64, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));
	DMA3COPY(tileTiles, OBJ_BASE_ADR + 96, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));

	DMA3COPY(game_backdropPal, &BG_COLORS[16 * 4], DMA16 | DMA_IMMEDIATE | game_backdropPalLen);
	DMA3COPY(game_backdropTiles, TILE_BASE_ADR(0) + 64, DMA16 | DMA_IMMEDIATE | game_backdropTilesLen);

	DMA3COPY(framePal, &OBJ_COLORS[16 * 4], DMA16 | DMA_IMMEDIATE | framePalLen);
	DMA3COPY(frameTiles, TILE_BASE_ADR(4) + 128 * 32, DMA16 | DMA_IMMEDIATE | frameTilesLen);

	clearSpriteTable();
	board.activeBlockL.raw.a = 0x4000;
	board.activeBlockL.raw.b = 0x0088;
	board.activeBlockL.raw.c = 0x0000;
	board.activeBlockR.raw.a = 0x4000;
	board.activeBlockR.raw.b = 0x0098;
	board.activeBlockR.raw.c = 0x0000;
	insertSprite(&board.activeBlockL, 0);
	insertSprite(&board.activeBlockR, 1);
	writeSpriteTable();

	resetBackdrop();

	REG_BG0CNT = CHAR_BASE(0) | SCREEN_BASE(1);
	REG_BG0HOFS = -8;
	REG_BG0VOFS = -24;
	REG_BG3CNT = CHAR_BASE(0) | SCREEN_BASE(2);
	REG_DISPCNT = MODE_0 | BG0_ON | BG3_ON | OBJ_ON | OBJ_1D_MAP;

	resetBoard();
}

void gameBoardDeinit() {
}

void gameBoardFrame(u32 framecount) {
	scanKeys();
	u16 keys = keysDown();

	if (keys & KEY_UP) {
		--board.activeY;
		if (board.activeY < 0) {
			board.activeY = 0;
		}
	}

	if (keys & KEY_DOWN) {
		++board.activeY;
		if (board.activeY >= GAMEBOARD_ROWS) {
			board.activeY = GAMEBOARD_ROWS - 1;
		}
	}

	if (keys & KEY_A) {
		layBlock();
	}

	board.activeBlockL.y = board.activeBlockR.y = 160 - 8 - 8 * GAMEBOARD_ROWS + (board.activeY << 3);

	drawBoard();
	updateSprite(&board.activeBlockL, 0);
	updateSprite(&board.activeBlockR, 1);
	writeSpriteTable();
}