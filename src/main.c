#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "audio.h"
#include "intro.h"
#include "runloop.h"

int main(void) {
	irqInit();
	soundInit();

	irqEnable(IRQ_VBLANK);
	irqSet(IRQ_VBLANK, soundFrame);
	REG_IME = 1;

	VBlankIntrWait();
	setRunloop(&intro);

	for (;;) {
		VBlankIntrWait();
		incrementRunloop();
	}

	return 0;
}