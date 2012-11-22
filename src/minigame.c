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
#include "util.h"

#include "bug.h"
#include "bullet.h"
#include "game-backdrop.h"
#include "pcb.h"
#include "spaceship.h"

// ABANDON HOPE, ALL YE WHO ENTER HERE
// This logic is horribly hard-coded and I am very sorry.

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

static s32 DIV16[278];
static s32 M7_D = 80;
static s32 M7_W = ((240 - 72 + 8) >> 1);

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
			.palette = 1,
			.priority = 3,
			.transformed = 0
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
	Coordinates currentCoords;
	int active;
	int dead;
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
			.palette = 2,
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

typedef struct Bullet {
	AffineSprite sprite;
	Coordinates coords;
} Bullet;

static Bullet friendlyBullets[32];
static u32 friendlyCooldown = 0;
static int activeBullets;

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

static int killstreak = 0;

static void switchState(int nextState, u32 framecount) {
	state = nextState;
	startFrame = framecount;
}

static void hideMinigame(void) {
	clearBlock((u16*) TILE_BASE_ADR(2), 186, 136, 64, 16);
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

static inline int bulletHit(Bullet* bullet, Bug* bug) {
	int bugDiffX = bug->currentCoords.x + (bullet->coords.x >> 6);
	int bugDiffY = 3 * bug->currentCoords.y - (bullet->coords.y >> 4);
	int bugDiffZ = (192 << 8) + (bullet->coords.z << 2) + bug->currentCoords.z;
	int scatter = bullet->coords.z >> 10;
	int scatterZ = 128 * scatter;
	int scatterX = 4 * scatter;
	int scatterY = 16 * scatter;
	if (bugDiffZ < -(0x1000 - scatterZ) || bugDiffZ > 0x1000 - scatterZ) {
		return 0;
	}
	if (bugDiffX < -(64 - scatterX) || bugDiffX > 64 - scatterX) {
		return 0;
	}
	if (bugDiffY < -(192 - scatterY) || bugDiffY > 192 - scatterY) {
		return 0;
	}

	return 1;
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
	bug.dead = 0;
	bug.sprite.sprite.transformed = 1;
	s32 seed = rand();
	genOctant(seed >> 12, &bug.coords);
	genOctant(seed >> 15, &bug.fakeoutCoords);
}

static void updateBug(void) {
	if (bug.dead > 63) {
		bug.dead = 0;
		bug.sprite.sprite.transformed = 0;
		REG_BLDALPHA = 0;
		bug.active = 0;
		updateSprite(&bug.sprite.sprite, bug.sprite.id);
	}
	if (!bug.active) {
		if ((rand() & 0xF0000) == 0xF0000) {
			generateBug();
		} else {
			return;
		}
	}
	s32 advance = offsets.z - bug.coords.z;
	// TODO: clean this up
	bug.sprite.affine.sX = bug.sprite.affine.sY = 2048 + (advance >> 4);
	s32 period = bug.sprite.affine.sX - 256;
	s32 x = period > 0 ? (bug.coords.x * (2048 - period) + bug.fakeoutCoords.x * period) >> 11 : bug.coords.x;
	s32 y = period > 0 ? (bug.coords.y * (2048 - period) + bug.fakeoutCoords.y * period) >> 11 : bug.coords.y;
	y += (bug.dead * bug.dead * advance) >> 18;
	bug.currentCoords.x = x;
	bug.currentCoords.y = y;
	bug.currentCoords.z = advance;
	bug.sprite.sprite.y = 24 + ((((((y >> 6) - y) >> 3) + 32) * (-advance >> 9)) >> 6);
	bug.sprite.sprite.x = 56 + (((((-offsets.x >> 6) - x) >> 3) * (-advance >> 9)) >> 6);
	unsigned int blend = 0;
	if (bug.dead) {
		blend = -bug.dead >> 2;
		bug.sprite.sprite.transformed ^= 1;
		++bug.dead;
	} else {
		bug.sprite.sprite.base ^= 0xC;
	}
	if (bug.sprite.affine.sX < 128) {
		bug.sprite.sprite.transformed = 0;
		blend = 0;
		bug.active = 0;
		if (!bug.dead) {
			killstreak = 0;
		}
	} else if (bug.sprite.affine.sX < 256) {
		bug.active = 2;
		if (!bug.dead) {
			bug.sprite.sprite.transformed ^= 1;
		}
		blend += ((bug.sprite.affine.sX - 128) >> 3);
	} else {
		blend += 0xF - (bug.sprite.affine.sX >> 7);
	}
	if (blend < 0 || blend > 0xF) {
		blend = 0;			
	}
	REG_BLDALPHA = blend;
	updateSprite(&bug.sprite.sprite, bug.sprite.id);
	ObjAffineSet(&bug.sprite.affine, affineTable(1), 1, 8);
}

static void fireFriendly(u32 framecount) {
	int i;
	// We must prepend the sprite to get layer ordering right
	if (activeBullets == currentParams.bulletsMax || friendlyCooldown + currentParams.bulletCooldown > framecount) {
		return;
	}
	friendlyCooldown = framecount;
	for (i = activeBullets; i > 0; --i) {
		Bullet* bullet = &friendlyBullets[i];
		Bullet* prevBullet = &friendlyBullets[i - 1];
		int id = bullet->sprite.id;
		int transformGroup = bullet->sprite.sprite.transformGroup;
		bullet->coords = prevBullet->coords;
		bullet->sprite = prevBullet->sprite;
		bullet->sprite.id = id;
		bullet->sprite.sprite.transformGroup = transformGroup;
		updateSprite(&bullet->sprite.sprite, bullet->sprite.id);
	}

	Bullet* bullet = &friendlyBullets[0];
	bullet->coords.x = offsets.x;
	bullet->coords.y = offsets.y;
	bullet->coords.z = 0;
	bullet->sprite.sprite.transformed = 1;
	bullet->sprite.sprite.disable = 0;
	updateSprite(&bullet->sprite.sprite, bullet->sprite.id);
	++activeBullets;
}

static void updateBullets(void) {
	int i;
	for (i = 0; i < activeBullets; ++i) {
		Bullet* bullet = &friendlyBullets[i];
		if (bullet->sprite.id) {
			bullet->coords.z -= 256;
			if (bullet->coords.z < -8192) {
				bullet->sprite.sprite.transformed = 0;
				bullet->sprite.sprite.disable = 1;
				--activeBullets;
			} else if (!bug.dead && bug.active == 1 && bulletHit(bullet, &bug)) {
				bug.dead = 1;
				++killstreak;
				board.score += killstreak;
				--board.bugs;
				updateScore();
				int inner;
				for (inner = i + 1; inner > activeBullets; ++inner) {
					bullet = &friendlyBullets[i];
					Bullet* prevBullet = &friendlyBullets[i - 1];
					int id = prevBullet->sprite.id;
					int transformGroup = prevBullet->sprite.sprite.transformGroup;
					prevBullet->coords = bullet->coords;
					prevBullet->sprite = bullet->sprite;
					prevBullet->sprite.id = id;
					prevBullet->sprite.sprite.transformGroup = transformGroup;
					updateSprite(&prevBullet->sprite.sprite, prevBullet->sprite.id);
				}
				bullet->sprite.sprite.transformed = 0;
				bullet->sprite.sprite.disable = 1;
				--activeBullets;
			} else {
				bullet->sprite.affine.sX = bullet->sprite.affine.sY = 256 - (bullet->coords.z >> 3);
				bullet->sprite.sprite.base ^= 2;
				bullet->sprite.sprite.y = (((82 - (bullet->coords.y >> 8) + (offsets.y >> 9)) * ((1024 << 5) + bullet->coords.z) + (-((128 + (bullet->coords.y << 1) - offsets.y) << 15) * bullet->coords.z))) >> 15;
				bullet->sprite.sprite.x = 80 - (((bullet->coords.x >> 11) - (offsets.x >> 12)) * ((256 << 5) + bullet->coords.z) >> 13);
				ObjAffineSet(&bullet->sprite.affine, affineTable(bullet->sprite.sprite.transformGroup), 1, 8);
			}
			updateSprite(&bullet->sprite.sprite, bullet->sprite.id);
		}
	}
}

void minigameInit(u32 framecount) {
	int i;
	for(i = 0; i < 160; ++i) {
		if (i == 66) {
			continue;
		}
		DIV16[i] = ((1 << 24) / (i - 66)) >> 8;
	}

	spaceship.sprite.sprite.doublesize = 1;
	spaceship.sprite.sprite.transformGroup = 0;
	spaceship.sprite.id = appendSprite(&spaceship.sprite.sprite);

	activeBullets = 0;
	for (i = 0; i < currentParams.bulletsMax; ++i) {
		Bullet* bullet = &friendlyBullets[i];
		bullet->sprite.sprite.base = 224;
		bullet->sprite.sprite.palette = 3;
		bullet->sprite.sprite.size = 1;
		bullet->sprite.sprite.transformed = 0;
		bullet->sprite.sprite.transformGroup = 2 + i;
		bullet->sprite.sprite.disable = 1;
		bullet->sprite.sprite.priority = 3;
		if (bullet->sprite.id > 0) {
			updateSprite(&bullet->sprite.sprite, bullet->sprite.id);
		} else {
			bullet->sprite.id = appendSprite(&bullet->sprite.sprite);
		}
	}

	bug.sprite.sprite.transformGroup = 1;
	bug.sprite.id = appendSprite(&bug.sprite.sprite);
}

void showMinigame(u32 framecount) {
	fadeOffset = 16;
	irqSet(IRQ_HBLANK, m7);
	irqEnable(IRQ_HBLANK);

	DMA3COPY(pcbPal, &BG_COLORS[0], DMA16 | DMA_IMMEDIATE | (16 * 2));
	DMA3COPY(pcbTiles, TILE_BASE_ADR(1), DMA16 | DMA_IMMEDIATE | (pcbTilesLen >> 1));
	DMA3COPY(spaceshipPal, &OBJ_COLORS[16], DMA16 | DMA_IMMEDIATE | 16);
	DMA3COPY(bugPal, &OBJ_COLORS[16 * 2], DMA16 | DMA_IMMEDIATE | 16);
	DMA3COPY(bulletPal, &OBJ_COLORS[16 * 3], DMA16 | DMA_IMMEDIATE | 16);
	// Sigh. Maybe I should go back to 1-D mapping
	DMA3COPY(spaceshipTiles, TILE_BASE_ADR(4) + 0x1400, DMA16 | DMA_IMMEDIATE | (spaceshipTilesLen >> 2));
	DMA3COPY(spaceshipTiles + 0x20, TILE_BASE_ADR(4) + 0x1800, DMA16 | DMA_IMMEDIATE | (spaceshipTilesLen >> 2));
	DMA3COPY(bugTiles, TILE_BASE_ADR(4) + 0x1480, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bugTiles + (bugTilesLen >> 4), TILE_BASE_ADR(4) + 0x1880, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bugTiles + (bugTilesLen >> 3), TILE_BASE_ADR(4) + 0x1C80, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bugTiles + 3 * (bugTilesLen >> 4), TILE_BASE_ADR(4) + 0x2080, DMA16 | DMA_IMMEDIATE | (bugTilesLen >> 3));
	DMA3COPY(bulletTiles, TILE_BASE_ADR(4) + 0x1C00, DMA16 | DMA_IMMEDIATE | (bulletTilesLen >> 2));
	DMA3COPY(bulletTiles + (bulletTilesLen >> 3), TILE_BASE_ADR(4) + 0x2000, DMA16 | DMA_IMMEDIATE | (bulletTilesLen >> 2));

	BG_COLORS[0] = game_backdropPal[15];

	REG_DISPCNT = MODE_1 | BG0_ON | BG1_ON | BG2_ON | OBJ_ON | WIN0_ON | WIN1_ON;
	REG_BG2CNT = CHAR_BASE(1) | SCREEN_BASE(4) | 0xA003;
	REG_BLDCNT = 0x24EF;
	REG_WIN0V = 0x4098;
	REG_WIN1H = 0x08A8;
	REG_WIN1V = 0x1840;

	renderText("DEBUG", &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 186,
		.clipY = 136,
		.clipW = 64,
		.clipH = 16,
		.baseline = 0
	}, &largeFont);

	calcMap(0, 0, 16, 16, 0, 0);

	int x, y;
	for (y = 0; y < 16; ++y) {
		for (x = 0; x < GAMEBOARD_COLS + GAMEBOARD_DEADZONE; ++x) {
			((u16*) SCREEN_BASE_BLOCK(2))[x + y * 32 + 97] = 0;
		}
	}

	switchState(FLYING_INTRO, framecount);
	killstreak = 0;
	offsets.x = 0;
	offsets.y = -32 << 8;
	offsets.z = 0;
	spaceship.offsetY = -(64 << 9);
	spaceship.sprite.sprite.mode = 1;
	spaceship.sprite.sprite.transformed = 1;

	bug.sprite.sprite.transformed = 0;
	updateSprite(&bug.sprite.sprite, bug.sprite.id);
	ObjAffineSet(&spaceship.sprite.affine, affineTable(0), 1, 8);
	ObjAffineSet(&bug.sprite.affine, affineTable(1), 1, 8);
	writeSpriteTable();
}

void minigameFrame(u32 framecount) {
	scanKeys();
	u16 keys = keysDown();

	if (keys & KEY_B && board.bugs < currentParams.bugShuntThreshold) {
		irqDisable(IRQ_HBLANK);
		hideMinigame();
		gameBoard.frame = gameBoardFrame;
		gameBoardSetup(framecount);
	}

	offsets.z -= currentParams.bugSpeed;

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

		if (keys & KEY_A) {
			fireFriendly(framecount);
		}
		updateBug();
	};

	camPos.x = (256 << 8) + offsets.x;
	camPos.y = (64 << 8) + offsets.y;
	camPos.z = (256 << 8) + offsets.z;
	int shipOffsetY = spaceship.offsetY + offsets.y;

	spaceship.sprite.affine.theta = -offsets.x >> 2;
	spaceship.sprite.affine.sX = spaceship.sprite.affine.sY = (shipOffsetY >> 8) + 256;
	spaceship.sprite.sprite.x = 56 - (offsets.x >> 11);
	spaceship.sprite.sprite.y = 72 - (shipOffsetY >> 9);
	updateSprite(&spaceship.sprite.sprite, spaceship.sprite.id);
	ObjAffineSet(&spaceship.sprite.affine, affineTable(0), 1, 8);
	updateBullets();
	writeSpriteTable();
}

// From Tonc
__attribute__((section(".iwram"), long_call))
static void m7() {
	int vcount = REG_VCOUNT;
	if (vcount >= 152 || vcount < 16) {
		return;
	}
	s32 lcf, lxr, lyr;

	s16 fade = bgFade[vcount] + fadeOffset;
	if (fade > 0xF) {
		REG_BLDY = 0xF;
	} else {
		REG_BLDY = fade;
	}

	lcf = camPos.y * DIV16[vcount] >> 12;
	
	REG_BG2PA = lcf >> 4;

	// Horizontal offset
	lxr = M7_W * (lcf >> 4);
	REG_BG2X = camPos.x - lxr;

	// Vertical offset
	lyr = (M7_D * lcf) >> 4; 
	REG_BG2Y = camPos.z - lyr;

	REG_IF |= IRQ_HBLANK;
}