#include "m7.h"

#include <gba_interrupt.h>
#include <gba_video.h>

extern void asmM7(void);

#if 0
// From Tonc
IWRAM_CODE static void m7() {
	int vcount = REG_VCOUNT;
	if (vcount >= 227) {
		vcount = 0;
	} else if (vcount >= 160) {
		return;
	}

	s32 lcf, lxr, lyr;

	s16 fade = m7Context.bgFade[vcount] + m7Context.fadeOffset;
	if (fade > 0x10) {
		REG_BLDY = 0x10;
	} else {
		REG_BLDY = fade;
	}

	lcf = m7Context.y * m7Context.div16[vcount] >> 12;
	
	REG_BG2PA = lcf >> 4;

	// Horizontal offset
	lxr = m7Context.w * (lcf >> 4);
	REG_BG2X = m7Context.x - lxr;

	// Vertical offset
	lyr = (m7Context.d * lcf) >> 4; 
	REG_BG2Y = m7Context.z - lyr;

	REG_IF |= IRQ_HBLANK;
}
#endif

void enableMode7(int enable) {
	if (enable) {
		irqSet(IRQ_HBLANK, asmM7);
		irqEnable(IRQ_HBLANK);
	} else {
		irqDisable(IRQ_HBLANK);
	}
}