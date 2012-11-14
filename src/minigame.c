#include "minigame.h"

#include <gba_affine.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_video.h>

#include "gameboard.h"
#include "sprite.h"
#include "text.h"

#include "pcb.h"
#include "spaceship.h"

static void m7();

static struct CameraPosition {
	s32 x, y, z;
} camPos, offsets;

static s32 gCos = 256;
static s32 gSin = 0;
static s32 DIV16[278];
static s32 M7_D = 80;

static struct {
	Sprite sprite;
	int id;
	ObjAffineSource affine;
} spaceship = {
	.sprite = {
		.x = 56,
		.y = 72,
		.base = 160,
		.shape = 1,
		.size = 2,
		.palette = 6,
		.transformed = 1
	},
	.id = -1,
	.affine = {
		.sX = 1 << 8,
		.sY = 1 << 8,
		.theta = 0
	}
};

static s16 bgFade[160] = {
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x0000,
	0x000F,

	0x000F,
	0x000F,
	0x000F,
	0x000F,
	0x000E,
	0x000E,
	0x000E,
	0x000D,
	0x000D,
	0x000D,
	0x000C,
	0x000C,
	0x000C,
	0x000B,
	0x000B,
	0x000A,
	0x000A,
	0x0008,
	0x0008,
	0x0007,
	0x0007,
	0x0006,
	0x0006,
	0x0005,
	0x0004,
	0x0003,
	0x0002,
	0x0001
};

static void hideMinigame(void) {
	int x, y;
	for (y = 17; y < 20; ++y) {
		for (x = 20; x < 32; ++x) {
			((u16*) SCREEN_BASE_BLOCK(3))[x + y * 32] = 0;
		}
	}
}

static void calcMap(int startX, int startY, int endX, int endY, int xOffset, int yOffset) {
	startX <<= 1;
	endX <<= 1;
	startY <<= 2;
	endY <<= 2;
	int x, y, offset;
	int xcell, ycell;
	for (y = startY; y < endY; ++y) {
		ycell = (y >> 2) + yOffset;
		for (x = startX; x < endX; ++x) {
			xcell = (x >> 1) + xOffset;
			offset = 3 + ((7 * (9 * xcell + 5) * (7 * ycell + 3) * xcell * ycell) >> 6);
			offset = ((x & 1) + (offset << 1)) & 7;
			((u16*) SCREEN_BASE_BLOCK(4))[x + y * 32] = (offset << 1) | (offset << 9) | 0x100 | ((y & 3) << 4) | ((y & 3) << 12);
		}
	}
}

void minigameInit() {
	DMA3COPY(pcbPal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 2));
	DMA3COPY(pcbTiles, TILE_BASE_ADR(1), DMA16 | DMA_IMMEDIATE | (pcbTilesLen >> 1));
	DMA3COPY(spaceshipPal, &OBJ_COLORS[16 * 6], DMA16 | DMA_IMMEDIATE | 16);
	DMA3COPY(spaceshipTiles, TILE_BASE_ADR(4) + 0x1400, DMA16 | DMA_IMMEDIATE | (spaceshipTilesLen >> 2));
	// Sigh. Maybe I should go back to 1-D mapping
	DMA3COPY(spaceshipTiles + 0x20, TILE_BASE_ADR(4) + 0x1800, DMA16 | DMA_IMMEDIATE | (spaceshipTilesLen >> 2));

	REG_DISPCNT = MODE_1 | BG0_ON | BG1_ON | BG2_ON | OBJ_ON | WIN0_ON;
	REG_BG2CNT = CHAR_BASE(1) | SCREEN_BASE(4) | 0xA002;
	REG_BLDCNT = 0x00CF;
	REG_WIN0V = 0x4098;

	irqSet(IRQ_HBLANK, m7);
	irqEnable(IRQ_HBLANK);

	mapText(SCREEN_BASE_BLOCK(3), 20, 32, 17, 20, 5);

	calcMap(0, 0, 16, 16, 0, 0);

	int i;
	for(i = 0; i < 160; ++i) {
		DIV16[i] = ((1 << 24) / (i - 64)) >> 8;
	}

	if (spaceship.id < 0) {
		spaceship.sprite.doublesize = 1;
		spaceship.sprite.transformGroup = 0;
		spaceship.id = appendSprite(&spaceship.sprite);
	}
	ObjAffineSet(&spaceship.affine, affineTable(0), 1, 8);
	writeSpriteTable();
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

	if (~REG_KEYINPUT & KEY_LEFT) {
		offsets.x = offsets.x - 512 - (offsets.x >> 5);
	} else if (~REG_KEYINPUT & KEY_RIGHT) {
		offsets.x = offsets.x + 512 - (offsets.x >> 5);
	} else {
		offsets.x -= (offsets.x >> 4) + 16;
	}

	if (~REG_KEYINPUT & KEY_DOWN) {
		offsets.y = offsets.y - 384 - (offsets.y >> 5);
	} else if (~REG_KEYINPUT & KEY_UP) {
		offsets.y = offsets.y + 384 - (offsets.y >> 5);
	} else {
		offsets.y -= offsets.y >> 4;
	}

	offsets.z -= 256;

	if (!((offsets.z >> 9) & 0xF)) {
		int range = ((offsets.z >> 13) + 8) & 0xF;
		calcMap(0, range, 16, range + 1, 0, offsets.z >> 13);
	}

	camPos.x = (256 << 8) + offsets.x;
	camPos.y = (64 << 8) + offsets.y;
	camPos.z = (256 << 8) + offsets.z;

	spaceship.affine.theta = -offsets.x >> 2;
	spaceship.affine.sX = spaceship.affine.sY = (offsets.y >> 7) + 256;
	spaceship.sprite.x = 56 - (offsets.x >> 10);
	spaceship.sprite.y = 72 - (offsets.y >> 9);
	updateSprite(&spaceship.sprite, spaceship.id);
	ObjAffineSet(&spaceship.affine, affineTable(0), 1, 8);
	writeSpriteTable();
}

#define M7_W ((240 - 72 + 8) >> 1)

// From Tonc
__attribute__((section(".iwram"), long_call))
static void m7() {
	s32 lam, lcf, lsf, lxr, lyr;

	REG_BLDY = bgFade[REG_VCOUNT];

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