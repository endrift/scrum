#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "gameboard.h"
#include "runloop.h"

int main(void) {
	irqInit();

	REG_DISPCNT = LCDC_OFF;
	irqEnable(IRQ_VBLANK);
	REG_IME = 1;

	VBlankIntrWait();
	setRunloop(&gameBoard);

	for (;;) {
		VBlankIntrWait();
		incrementRunloop();
	}

	return 0;
}