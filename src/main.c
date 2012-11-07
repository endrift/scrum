#include <gba_interrupt.h>
#include <gba_systemcalls.h>

#include "gameboard.h"
#include "runloop.h"

int main(void) {
	irqInit();

	setRunloop(&gameBoard);

	irqEnable(IRQ_VBLANK);
	REG_IME = 1;

	for (;;) {
		VBlankIntrWait();
		incrementRunloop();
	}

	return 0;
}