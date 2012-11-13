#include "minigame.h"

#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_video.h>

#include "gameboard.h"

static void m7();

static struct CameraPosition {
	s32 x, y, z;
} camPos;

static s32 gCos = 181;
static s32 gSin = 181;
static s32 DIV16[278];
static s32 M7_D = 512;

void minigameInit() {
	REG_DISPCNT = MODE_1 | BG0_ON | BG1_ON | BG2_ON | OBJ_ON | WIN0_ON;
	REG_BG2CNT = CHAR_BASE(0) | SCREEN_BASE(4) | 0xA002;
	irqSet(IRQ_HBLANK, m7);
	irqEnable(IRQ_HBLANK);
	camPos.x = 256 << 8;
	camPos.y = 16 << 8;
	camPos.z = 256 << 8;

	int x, y;
	for (y = 0; y < 64; ++y) {
		for (x = 0; x < 64; ++x) {
			((u8*) SCREEN_BASE_BLOCK(4))[x + y * 64] = 2;
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
		gameBoard.frame = gameBoardFrame;
		gameBoardSetup();
	}

	camPos.z -= 1 << 7;
	camPos.x += 1 << 7;
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