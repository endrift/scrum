#include "minigame.h"

#include <gba_affine.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_video.h>

#include "gameboard.h"
#include "sprite.h"
#include "text.h"

#include "bug.h"
#include "game-backdrop.h"
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

typedef struct AffineSprite {
	Sprite sprite;
	int id;
	ObjAffineSource affine;
} AffineSprite;

static AffineSprite spaceship = {
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

static AffineSprite bugs[5] = {
	{ .id = -1 },
	{ .id = -1 },
	{ .id = -1 },
	{ .id = -1 },
	{ .id = -1 }
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
	0x0001,
	0x0001,
	0x0001,
	0x0002,
	0x0002,
	0x0002,
	0x0003,
	0x0003,
	0x0003,
	0x0004,
	0x0004,
	0x0004,
	0x0005,
	0x0005,
	0x0005,
	0x0006,
	0x0006,
	0x0006,
	0x0007,
	0x0007,
	0x0007,
	0x0008,
	0x0008,
	0x0008,
	0x0009,
	0x0009,
	0x0009,
	0x000A,
	0x000A,
	0x000A,
	0x000B,
	0x000B,
	0x000B,
	0x000C,
	0x000C,
	0x000C,
	0x000D,
	0x000D,
	0x000D,
	0x000E,
	0x000E,
	0x000E,
	0x000F,
	0x000F,
	0x000F,
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
	spaceship.sprite.transformed = 0; // Doublesize == disabled, so this disables it
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
	DMA3COPY(bugPal, &OBJ_COLORS[16 * 7], DMA16 | DMA_IMMEDIATE | 16);
	// Sigh. Maybe I should go back to 1-D mapping
	DMA3COPY(spaceshipTiles, TILE_BASE_ADR(4) + 0x1400, DMA16 | DMA_IMMEDIATE | (spaceshipTilesLen >> 2));
	DMA3COPY(spaceshipTiles + 0x20, TILE_BASE_ADR(4) + 0x1800, DMA16 | DMA_IMMEDIATE | (spaceshipTilesLen >> 2));
	DMA3COPY(bugTiles, TILE_BASE_ADR(4) + 0x1480, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bugTiles + 0x20, TILE_BASE_ADR(4) + 0x1880, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bugTiles + 0x40, TILE_BASE_ADR(4) + 0x1C80, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bugTiles + 0x60, TILE_BASE_ADR(4) + 0x2080, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	BG_COLORS[0] = game_backdropPal[15];


	REG_DISPCNT = MODE_1 | BG0_ON | BG1_ON | BG2_ON | OBJ_ON | WIN0_ON | WIN1_ON;
	REG_BG2CNT = CHAR_BASE(1) | SCREEN_BASE(4) | 0xA003;
	REG_BLDCNT = 0x24EF;
	REG_WIN0V = 0x4098;
	REG_WIN1H = 0x08A8;
	REG_WIN1V = 0x1840;

	irqSet(IRQ_HBLANK, m7);
	irqEnable(IRQ_HBLANK);

	mapText(SCREEN_BASE_BLOCK(3), 20, 32, 17, 20, 5);


	calcMap(0, 0, 16, 16, 0, 0);

	int x, y;
	for (y = 0; y < 16; ++y) {
		for (x = 0; x < GAMEBOARD_COLS + GAMEBOARD_DEADZONE; ++x) {
			((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32 + 97] = 0;
		}
	}

	int i;
	for(i = 0; i < 160; ++i) {
		if (i == 66) {
			continue;
		}
		DIV16[i] = ((1 << 24) / (i - 66)) >> 8;
	}

	if (spaceship.id < 0) {
		spaceship.sprite.doublesize = 1;
		spaceship.sprite.transformGroup = 0;
		spaceship.id = appendSprite(&spaceship.sprite);
	} else {
		spaceship.sprite.transformed = 1;
	}

	if (bugs[0].id < 0) {
		bugs[0].sprite.doublesize = 1;
		bugs[0].sprite.x = 56;
		bugs[0].sprite.y = 40;
		bugs[0].sprite.mode = 1;
		bugs[0].sprite.base = 164;
		bugs[0].sprite.shape = 0;
		bugs[0].sprite.size = 2;
		bugs[0].sprite.palette = 7;
		bugs[0].sprite.priority = 3;
		bugs[0].sprite.transformed = 1;
		bugs[0].sprite.transformGroup = 1;
		bugs[0].affine.sX = 1 << 12;
		bugs[0].affine.sY = 1 << 12;
		bugs[0].affine.theta = 0;
		bugs[0].id = appendSprite(&bugs[0].sprite);
	}
	ObjAffineSet(&spaceship.affine, affineTable(0), 1, 8);
	ObjAffineSet(&bugs[0].affine, affineTable(1), 1, 8);
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
	spaceship.affine.sX = spaceship.affine.sY = (offsets.y >> 8) + 256;
	spaceship.sprite.x = 56 - (offsets.x >> 10);
	spaceship.sprite.y = 72 - (offsets.y >> 9);
	bugs[0].sprite.y = 40 + (offsets.z >> 11);
	bugs[0].affine.sX = bugs[0].affine.sY = 2048 + (M7_D >> 3) * (offsets.z >> 8);
	unsigned int blend;
	if (bugs[0].affine.sX < 128) {
		bugs[0].sprite.transformed = 0;
		blend = 0;
	} else if (bugs[0].affine.sX < 192) {
		blend = ((bugs[0].affine.sX - 128) >> 2);
	} else {
		blend = 0xF - (bugs[0].affine.sX >> 7);
	}
	if (blend < 0) {
		blend = 0;			
	}
	REG_BLDALPHA = blend;
	updateSprite(&spaceship.sprite, spaceship.id);
	updateSprite(&bugs[0].sprite, bugs[0].id);
	ObjAffineSet(&spaceship.affine, affineTable(0), 1, 8);
	ObjAffineSet(&bugs[0].affine, affineTable(1), 1, 8);
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