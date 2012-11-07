#include "runloop.h"

Runloop* currentRunloop = 0;
u32 frame = 0;

void setRunloop(Runloop* runloop) {
	if (currentRunloop) {
		currentRunloop->deinit();
	}
	currentRunloop = runloop;
	currentRunloop->init();
}

void incrementRunloop() {
	currentRunloop->frame(frame);
	++frame;
}