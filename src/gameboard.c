#include "gameboard.h"

#include <gba_dma.h>
#include <gba_input.h>
#include <gba_sprites.h>
#include <gba_video.h>

#include "sprite.h"

#include "tile-palette.h"
#include "tile-data.h"

Runloop gameBoard = {
	.init = gameBoardInit,
	.deinit = gameBoardDeinit,
	.frame = gameBoardFrame
};

typedef struct GameBoard {
	struct Row {
		u8 color[GAMEBOARD_COLS];
		u8 width;
	} rows[GAMEBOARD_ROWS + GAMEBOARD_DEADZONE];

	Sprite activeBlock;
	int activeY;
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

static void resetBoard(void) {
	int x, y;
	for (y = 0; y < GAMEBOARD_ROWS; ++y) {
		board.rows[y].width = 0;
	}
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

	clearSpriteTable();
	board.activeBlock.raw.a = 0x4000;
	board.activeBlock.raw.b = 0x4088;
	board.activeBlock.raw.c = 0x0000;
	insertSprite(&board.activeBlock, 0);
	writeSpriteTable();

	REG_BG0CNT = CHAR_BASE(0) | SCREEN_BASE(1);
	REG_BG0HOFS = -8;
	REG_BG0VOFS = -24;
	REG_DISPCNT = MODE_0 | BG0_ON | OBJ_ON;

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
		if (board.rows[board.activeY].width < GAMEBOARD_COLS) {
			++board.rows[board.activeY].width;
		}
	}

	board.activeBlock.y = 160 - 8 - 8 * GAMEBOARD_ROWS + (board.activeY << 3);

	drawBoard();
	updateSprite(&board.activeBlock, 0);
	writeSpriteTable();
}