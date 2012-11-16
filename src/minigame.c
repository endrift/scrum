#include "minigame.h"

#include <gba_affine.h>
#include <gba_dma.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_video.h>

#include "gameboard.h"
#include "rng.h"
#include "sprite.h"
#include "text.h"

#include "bug.h"
#include "game-backdrop.h"
#include "pcb.h"
#include "spaceship.h"

static void m7();

static enum {
	FLYING_INTRO,
	FLYING_GAMEPLAY
} state = FLYING_INTRO;

static u32 startFrame;

typedef struct Coordinates {
	s32 x, y, z;
} Coordinates;
static Coordinates camPos, offsets;

static s32 gCos = 256;
static s32 gSin = 0;
static s32 DIV16[278];
static s32 M7_D = 80;

typedef struct AffineSprite {
	Sprite sprite;
	int id;
	ObjAffineSource affine;
} AffineSprite;

static struct {
	AffineSprite sprite;
	s32 offsetY;
} spaceship = {
	.sprite = {
		.sprite = {
			.x = 56,
			.y = 72,
			.mode = 1,
			.base = 160,
			.shape = 1,
			.size = 2,
			.palette = 6,
			.priority = 3,
			.transformed = 1
		},
		.id = -1,
		.affine = {
			.sX = 1 << 8,
			.sY = 1 << 8,
			.theta = 0
		}
	}
};


typedef struct Bug {
	AffineSprite sprite;
	Coordinates coords;
	Coordinates fakeoutCoords;
	int active;
} Bug;

static Bug bug = {
	.sprite = {
		.sprite = {
			.disable = 1,
			.x = 56,
			.y = 40,
			.mode = 1,
			.base = 164,
			.shape = 0,
			.size = 2,
			.palette = 7,
			.priority = 3,
			.transformed = 0
		},
		.affine = {
			.sX = 1 << 8,
			.sY = 1 << 8,
			.theta = 0
		},
		.id = -1
	},
	.coords = {
		.x = 0 << 8,
		.y = 0 << 8,
		.z = 0
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

static s16 fadeOffset = 0;

static void switchState(int nextState, u32 framecount) {
	state = nextState;
	startFrame = framecount;
}

static void hideMinigame(void) {
	int x, y;
	for (y = 17; y < 20; ++y) {
		for (x = 20; x < 32; ++x) {
			((u16*) SCREEN_BASE_BLOCK(3))[x + y * 32] = 0;
		}
	}
	spaceship.sprite.sprite.transformed = 0; // Doublesize == disabled, so this disables it
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

static void genOctant(int seed, Coordinates* coords) {
		switch (seed & 0x7) {
		case 0x0:
			coords->x = -256;
			coords->y = -256;
			break;
		case 0x1:
			coords->x = 0;
			coords->y = -256;
			break;
		case 0x2:
			coords->x = 256;
			coords->y = -256;
			break;
		case 0x3:
			coords->x = -256;
			coords->y = 0;
			break;
		case 0x4:
			coords->x = 256;
			coords->y = 0;
			break;
		case 0x5:
			coords->x = -256;
			coords->y = 256;
			break;
		case 0x6:
			coords->x = 0;
			coords->y = 256;
			break;
		case 0x7:
			coords->x = 256;
			coords->y = 256;
			break;
	}

}

static void generateBug(void) {
	bug.coords.z = offsets.z;
	bug.active = 1;
	bug.sprite.sprite.transformed = 1;
	s32 seed = rand();
	genOctant(seed >> 12, &bug.coords);
	genOctant(seed >> 15, &bug.fakeoutCoords);
}

static void updateBug(void) {
	if (!bug.active) {
		if ((rand() & 0x7F0000) == 0x7F0000) {
			generateBug();
		} else {
			return;
		}
	}
	s32 advance = offsets.z - bug.coords.z;
	s32 period = bug.sprite.affine.sX - 256;
	// TODO: clean this up
	bug.sprite.affine.sX = bug.sprite.affine.sY = 2048 + (advance >> 4);
	s32 x = period > 0 ? (bug.coords.x * (2048 - period) + bug.fakeoutCoords.x * period) >> 11 : bug.coords.x;
	s32 y = period > 0 ? (bug.coords.y * (2048 - period) + bug.fakeoutCoords.y * period) >> 11 : bug.coords.y;
	bug.sprite.sprite.y = 24 + ((((((y >> 6) - y) >> 3) + 32) * (-advance >> 9)) >> 6);
	bug.sprite.sprite.x = 56 + ((((3 * (-offsets.x >> 7) - x) >> 3) * (-advance >> 9)) >> 6);
	bug.sprite.sprite.base ^= 0xC;
	unsigned int blend;
	if (bug.sprite.affine.sX < 128) {
		bug.sprite.sprite.transformed = 0;
		blend = 0;
		bug.active = 0;
	} else if (bug.sprite.affine.sX < 256) {
		bug.sprite.sprite.transformed ^= 1;
		blend = ((bug.sprite.affine.sX - 128) >> 3);
	} else {
		blend = 0xF - (bug.sprite.affine.sX >> 7);
	}
	if (blend < 0 || blend > 0xF) {
		blend = 0;			
	}
	REG_BLDALPHA = blend;
	updateSprite(&bug.sprite.sprite, bug.sprite.id);
	ObjAffineSet(&bug.sprite.affine, affineTable(1), 1, 8);
}

void minigameInit(u32 framecount) {
	int i;
	for(i = 0; i < 160; ++i) {
		if (i == 66) {
			continue;
		}
		DIV16[i] = ((1 << 24) / (i - 66)) >> 8;
	}
	fadeOffset = 16;

	irqSet(IRQ_HBLANK, m7);
	irqEnable(IRQ_HBLANK);

	DMA3COPY(pcbPal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 2));
	DMA3COPY(pcbTiles, TILE_BASE_ADR(1), DMA16 | DMA_IMMEDIATE | (pcbTilesLen >> 1));
	DMA3COPY(spaceshipPal, &OBJ_COLORS[16 * 6], DMA16 | DMA_IMMEDIATE | 16);
	DMA3COPY(bugPal, &OBJ_COLORS[16 * 7], DMA16 | DMA_IMMEDIATE | 16);
	// Sigh. Maybe I should go back to 1-D mapping
	DMA3COPY(spaceshipTiles, TILE_BASE_ADR(4) + 0x1400, DMA16 | DMA_IMMEDIATE | (spaceshipTilesLen >> 2));
	DMA3COPY(spaceshipTiles + 0x20, TILE_BASE_ADR(4) + 0x1800, DMA16 | DMA_IMMEDIATE | (spaceshipTilesLen >> 2));
	DMA3COPY(bugTiles, TILE_BASE_ADR(4) + 0x1480, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bugTiles + (bugTilesLen >> 4), TILE_BASE_ADR(4) + 0x1880, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bugTiles + (bugTilesLen >> 3), TILE_BASE_ADR(4) + 0x1C80, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bugTiles + 3 * (bugTilesLen >> 4), TILE_BASE_ADR(4) + 0x2080, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	BG_COLORS[0] = game_backdropPal[15];

	REG_DISPCNT = MODE_1 | BG0_ON | BG1_ON | BG2_ON | OBJ_ON | WIN0_ON | WIN1_ON;
	REG_BG2CNT = CHAR_BASE(1) | SCREEN_BASE(4) | 0xA003;
	REG_BLDCNT = 0x24EF;
	REG_WIN0V = 0x4098;
	REG_WIN1H = 0x08A8;
	REG_WIN1V = 0x1840;

	mapText(SCREEN_BASE_BLOCK(3), 20, 32, 17, 20, 5);

	calcMap(0, 0, 16, 16, 0, 0);

	int x, y;
	for (y = 0; y < 16; ++y) {
		for (x = 0; x < GAMEBOARD_COLS + GAMEBOARD_DEADZONE; ++x) {
			((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32 + 97] = 0;
		}
	}

	startFrame = framecount;
	state = FLYING_INTRO;
	offsets.x = 0;
	offsets.y = -32 << 8;
	offsets.z = 0;
	spaceship.offsetY = -(64 << 9);
	spaceship.sprite.sprite.mode = 1;

	if (spaceship.sprite.id < 0) {
		spaceship.sprite.sprite.doublesize = 1;
		spaceship.sprite.sprite.transformGroup = 0;
		spaceship.sprite.id = appendSprite(&spaceship.sprite.sprite);
	} else {
		spaceship.sprite.sprite.transformed = 1;
	}

	if (bug.sprite.id < 0) {
		bug.sprite.sprite.transformGroup = 1;
		bug.sprite.id = appendSprite(&bug.sprite.sprite);
	} else {
		bug.sprite.sprite.transformed = 0;
		updateSprite(&bug.sprite.sprite, bug.sprite.id);
	}
	ObjAffineSet(&spaceship.sprite.affine, affineTable(0), 1, 8);
	ObjAffineSet(&bug.sprite.affine, affineTable(1), 1, 8);
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

	offsets.z -= 256;

	if (!((offsets.z >> 9) & 0xF)) {
		int range = ((offsets.z >> 13) + 8) & 0xF;
		calcMap(0, range, 16, range + 1, 0, offsets.z >> 13);
	}

	switch (state) {
	case FLYING_INTRO:
		if ((framecount - startFrame) >= 64) {
			switchState(FLYING_GAMEPLAY, framecount);
			spaceship.sprite.sprite.mode = 0;
			generateBug();
		} else {
			REG_BLDALPHA = (framecount - startFrame) >> 2;
			spaceship.offsetY -= spaceship.offsetY >> 5;
			offsets.y -= offsets.y >> 4;
			if (((framecount - startFrame) & 0x3) == 0x3) {
				--fadeOffset;
			}
		}
		break;
	case FLYING_GAMEPLAY:
		if (~REG_KEYINPUT & KEY_LEFT) {
			offsets.x = offsets.x - 512 - (offsets.x >> 5);
		} else if (~REG_KEYINPUT & KEY_RIGHT) {
			offsets.x = offsets.x + 512 - (offsets.x >> 5);
		} else {
			offsets.x -= (offsets.x >> 4);
		}

		if (~REG_KEYINPUT & KEY_DOWN) {
			offsets.y = offsets.y - 384 - (offsets.y >> 5);
		} else if (~REG_KEYINPUT & KEY_UP) {
			offsets.y = offsets.y + 384 - (offsets.y >> 5);
		} else {
			offsets.y -= offsets.y >> 4;
		}
		updateBug();
	};

	camPos.x = (256 << 8) + offsets.x;
	camPos.y = (64 << 8) + offsets.y;
	camPos.z = (256 << 8) + offsets.z;
	int shipOffsetY = spaceship.offsetY + offsets.y;

	spaceship.sprite.affine.theta = -offsets.x >> 2;
	spaceship.sprite.affine.sX = spaceship.sprite.affine.sY = (shipOffsetY >> 8) + 256;
	spaceship.sprite.sprite.x = 56 - (offsets.x >> 10);
	spaceship.sprite.sprite.y = 72 - (shipOffsetY >> 9);
	updateSprite(&spaceship.sprite.sprite, spaceship.sprite.id);
	ObjAffineSet(&spaceship.sprite.affine, affineTable(0), 1, 8);
	writeSpriteTable();
}

#define M7_W ((240 - 72 + 8) >> 1)

// From Tonc
__attribute__((section(".iwram"), long_call))
static void m7() {
	s32 lam, lcf, lsf, lxr, lyr;

	s16 fade = bgFade[REG_VCOUNT] + fadeOffset;
	if (fade > 0xF) {
		REG_BLDY = 0xF;
	} else {
		REG_BLDY = fade;
	}

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