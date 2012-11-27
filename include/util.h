#ifndef UTIL_H
#define UTIL_H

#include <gba_types.h>

inline void formatNumber(char* buffer, unsigned int len, unsigned int value);

void clearBlock(u16* base, int startX, int startY, int width, int height);

void byteCopy(void* dst, const void* src, int bsize);
void byteZero(void* dst, int bsize);

void hzero(u16* base, int hsize);

#endif