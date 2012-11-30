#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>
#include <maxmod.h>

#include "audio.h"
#include "intro.h"
#include "runloop.h"

int main(void) {
	irqInit();
	soundInit();

	irqEnable(IRQ_VBLANK);
	REG_IME = 1;

	VBlankIntrWait();
	setRunloop(&intro);

	for (;;) {
		VBlankIntrWait();
		mmFrame();
		incrementRunloop();
	}

	return 0;
}
