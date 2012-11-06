#include <gba_interrupt.h>
#include <gba_systemcalls.h>

#include "gameboard.h"

int main(void) {
	irqInit();

	gameBoardInit();

	irqEnable(IRQ_VBLANK);
	REG_IME = 1;

	for (;;) {
		VBlankIntrWait();
	}

	return 0;
}