#include "gameboard.h"

#include <gba_dma.h>
#include <gba_video.h>

#include "tile-palette.h"
#include "tile-data.h"

Runloop gameBoard = {
	.init = gameBoardInit,
	.deinit = gameBoardDeinit,
	.frame = gameBoardFrame
};

static GameBoard board;

static void drawBoard(void) {
	u16* mapData = SCREEN_BASE_BLOCK(1);

	int x, y;
	for (y = 0; y < GAMEBOARD_ROWS; ++y) {
		for (x = 0; x < board.rows[y].width; ++x) {
			mapData[x + y * 32] = 1 | CHAR_PALETTE(board.rows[y].color[x]);
		}
		for (; x < GAMEBOARD_COLS; ++x) {
			mapData[x + y * 32] = 0;
		}
	}
}

static void resetBoard(void) {
	int x, y;
	for (y = 0; y < GAMEBOARD_ROWS; ++y) {
		board.rows[y].width = y;
		for (x = 0; x < GAMEBOARD_COLS; ++x) {
			board.rows[y].color[x] = 0;
		}
	}
}

void gameBoardInit() {
	DMA3COPY(tile_bluePal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 4));
	DMA3COPY(tileTiles, TILE_BASE_ADR(0) + 32, DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));

	REG_BG0CNT = CHAR_BASE(0) | SCREEN_BASE(1);
	REG_BG0HOFS = -8;
	REG_BG0VOFS = -24;
	REG_DISPCNT = MODE_0 | BG0_ON;

	resetBoard();
}

void gameBoardDeinit() {
}

void gameBoardFrame(u32 framecount) {
	drawBoard();

	int y;
	for (y = 0; y < GAMEBOARD_ROWS; ++y) {
		board.rows[y].width = ((y + framecount / 60) & 15) + 1;
	}
}