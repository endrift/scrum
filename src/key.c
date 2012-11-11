#include "key.h"

void startRepeat(KeyContext* context, u32 frame, unsigned int keys) {
	context->active |= keys;
	int i;
	for (i = 0; keys && i < 12; ++i, keys >>= 1) {
		if (keys & 1) {
			context->next[i] = frame + context->startDelay;
		}
	}
}

void stopRepeat(KeyContext* context, int keys) {
	context->active &= ~keys;
}

void doRepeat(KeyContext* context, u32 frame) {
	int i;
	int keys = context->active;
	int repeat = 0;
	for (i = 0; keys && i < 12; ++i, keys >>= 1) {
		if (keys & 1 && frame >= context->next[i]) {
			repeat |= 1 << i;
			context->next[i] += context->repeatDelay;
		}
	}
	if (repeat) {
		context->repeatHandler(context, repeat);
	}
}