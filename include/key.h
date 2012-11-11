#ifndef KEY_H
#define KEY_H

#include <gba_types.h>

typedef struct KeyContext {
	u32 next[12];
	unsigned int active;
	unsigned int startDelay;
	unsigned int repeatDelay;
	void (*repeatHandler)(struct KeyContext* context, int keys);
} KeyContext;

void startRepeat(KeyContext* context, u32 frame, unsigned int keys);

void stopRepeat(KeyContext* context, int keys);

void doRepeat(KeyContext* context, u32 frame);

#endif