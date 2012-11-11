#ifndef UTIL_H
#define UTIL_H

#include <gba_types.h>

inline void formatNumber(char* buffer, unsigned int len, unsigned int value) {
	int i;
	for (i = 0; i < len; ++i) {
		buffer[len - i - 1] = '0' + (value % 10);
		value /= 10;
	}
}

inline void clearBlock(u16* base, int startX, int startY, int width, int height) {
	// TODO: put swizzling in a #define
	int x, y;
	for (y = startY; y < startY + height; ++y) {
		for (x = startX; x < startX + width; ++x) {
			int swizzled = (y & 0x7) * 4; // Row
			swizzled += (x & 0x7) >> 1; // Column
			swizzled += (x >> 3) * 0x20 + (y >> 3) * 0x400; // Cell
			base[swizzled >> 1] &= ~(0xF << (4 * (x & 3)));
		}
	}
}

#endif