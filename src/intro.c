#include "intro.h"

#include <gba_compression.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "gameboard.h"

#include "endrift.h"

static void introInit(u32 framecount);
static void introDeinit(void);
static void introFrame(u32 framecount);

Runloop intro = {
	.init = introInit,
	.deinit = introDeinit,
	.frame = introFrame
};

static u32 introStart = 0;

void introInit(u32 framecount) {
	introStart = framecount;

	REG_BLDALPHA = 0x0F0F; // Whoops, GBA.js doesn't support BLDY in modes 3 - 5
	REG_BLDCNT = 0x3F7F;

	LZ77UnCompVram((void*) endriftBitmap, (void*) VRAM);
}

void introDeinit(void) {
	RegisterRamReset(1 << 3);
}

void introFrame(u32 framecount) {
	REG_DISPCNT = MODE_3 | BG2_ON;
	if (framecount - introStart < 64) {
		REG_BLDALPHA = (framecount - introStart) >> 2 | 0x0F00;
	} else if (framecount - introStart > 160) {
		REG_DISPCNT = LCDC_OFF;
		setRunloop(&gameBoard);
	} else if (framecount - introStart > 120) {
		BG_COLORS[0] = 0x7FFF;
		REG_BLDALPHA = (framecount - introStart - 120) << 7 | 0x000F;
	}
}