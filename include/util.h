#ifndef UTIL_H
#define UTIL_H

#include <gba_types.h>

inline void formatNumber(char* buffer, unsigned int len, unsigned int value);

void clearBlock(u16* base, int startX, int startY, int width, int height);

void hzero(u16* base, int hsize);

#endif