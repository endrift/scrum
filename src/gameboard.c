#include <gba_dma.h>
#include <gba_video.h>

#include "tile-palette.h"
#include "tile-data.h"

void gameBoardInit() {
	DMA0COPY(tile_bluePal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 4));
	DMA0COPY(tileTiles, TILE_BASE_ADR(0), DMA16 | DMA_IMMEDIATE | (tileTilesLen >> 1));

	u16* screen = SCREEN_BASE_BLOCK(1);
	int i;
	for (i = 0; i < 32 * 32; ++i) {
		screen[i] = CHAR_PALETTE((i / 32) & 3);
	}

	REG_BG0CNT = CHAR_BASE(0) | SCREEN_BASE(1);
	REG_DISPCNT = MODE_0 | BG0_ON;
}