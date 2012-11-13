#include "minigame.h"

#include <gba_dma.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_video.h>

#include "gameboard.h"
#include "text.h"

#include "pcb.h"

static void m7();

static struct CameraPosition {
	s32 x, y, z;
} camPos;

static s32 gCos = 237;
static s32 gSin = 98;
static s32 DIV16[278];
static s32 M7_D = 256;

static void hideMinigame(void) {
	int x, y;
	for (y = 17; y < 20; ++y) {
		for (x = 20; x < 32; ++x) {
			((u16*) SCREEN_BASE_BLOCK(3))[x + y * 32] = 0;
		}
	}
}

void minigameInit() {
	DMA3COPY(pcbPal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 4));
	DMA3COPY(pcbTiles, TILE_BASE_ADR(1), DMA16 | DMA_IMMEDIATE | (pcbTilesLen >> 1));

	REG_DISPCNT = MODE_1 | BG0_ON | BG1_ON | BG2_ON | OBJ_ON | WIN0_ON;
	REG_BG2CNT = CHAR_BASE(1) | SCREEN_BASE(4) | 0x6002;
	irqSet(IRQ_HBLANK, m7);
	irqEnable(IRQ_HBLANK);
	camPos.x = 256 << 8;
	camPos.y = 64 << 8;
	camPos.z = 256 << 8;

	mapText(SCREEN_BASE_BLOCK(3), 20, 32, 17, 20);

	int x, y, offset;
	int xcell, ycell;
	for (y = 0; y < 32; ++y) {
		ycell = y >> 2;
		for (x = 0; x < 16; ++x) {
			xcell = x >> 1;
			offset = 1 - ((105 * xcell * ycell * (ycell - 4) * (xcell - 4)) >> 6);
			offset = ((x & 1) + (offset << 1)) & 7;
			((u16*) SCREEN_BASE_BLOCK(4))[x + y * 16] = (offset << 1) | (offset << 9) | 0x100 | ((y & 3) << 4) | ((y & 3) << 12);
		}
	}

	int i;
	for(i = 0; i < 160; ++i) {
		DIV16[i] = ((1 << 24) / (1 + i)) >> 8;
	}
}

void minigameFrame(u32 framecount) {
	scanKeys();
	u16 keys = keysDown();

	if (keys & KEY_B) {
		irqDisable(IRQ_HBLANK);
		hideMinigame();
		gameBoard.frame = gameBoardFrame;
		gameBoardSetup();
	}

	camPos.z -= gCos;
	camPos.x += gSin;
}

#define M7_W ((240 - 72 + 8) >> 1)

// From Tonc
__attribute__((section(".iwram"), long_call))
static void m7() {
	s32 lam, lcf, lsf, lxr, lyr;


	lam = camPos.y * DIV16[REG_VCOUNT] >> 12;
	lcf = lam * gCos >> 8;
	lsf = lam * gSin >> 8;

	REG_BG2PA = lcf >> 4;
	REG_BG2PC = lsf >> 4;

	// Horizontal offset
	lxr = M7_W * (lcf >> 4);
	lyr = (M7_D * lsf) >> 4;
	REG_BG2X = camPos.x - lxr + lyr;

	// Vertical offset
	lxr = M7_W * (lsf >> 4);
	lyr = (M7_D * lcf) >> 4; 
	REG_BG2Y = camPos.z - lxr - lyr;

	REG_IF |= IRQ_HBLANK;
}