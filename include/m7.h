#ifndef M7_H
#define M7_H

#include <gba_types.h>

typedef struct Mode7Context {
	s32 x, y, z;
	s32 div16[228];
	s32 d;
	s32 w;
	s16 bgFade[160];
	s16 fadeOffset;
} Mode7Context;

extern Mode7Context m7Context;

void enableMode7(int enable);

#endif