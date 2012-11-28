#include "text.h"

#include <gba_base.h>

#include <string.h>

#include "large_font.h"
#include "thin_font.h"

struct Font {
	unsigned int gridW;
	unsigned int gridH;
	int baseline;
	const u16* grid;
	const Metrics* metrics;
};

EWRAM_DATA const Glyph largeFontGlyphs[128] = {
	{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
	{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
	{ // space 
		.width = 8 
	},
	{ // !
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9
	},
	{}, {}, {},
	{ // %
		.clipLeft = 1,
		.clipWidth = 14,
		.width = 13,
	},
	{}, {}, {}, {}, {}, {}, {}, {},
	{ // .
		.clipLeft = 6,
		.clipWidth = 4,
		.width = 4
	},
	{},
	{ // 0
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // 1
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // 2
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // 3
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // 4
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // 5
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // 6
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // 7
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // 8
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // 9
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // :
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // ;
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // <
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // =
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // >
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // ?
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // @
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // A
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // B
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // C
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // D
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // E
		.clipLeft = 3,
		.clipWidth = 9,
		.width = 8,
	},
	{ // F
		.clipLeft = 3,
		.clipWidth = 9,
		.width = 8,
	},
	{ // G
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // H
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // I
		.clipLeft = 5,
		.clipWidth = 6,
		.width = 5,
	},
	{ // J
		.clipLeft = 4,
		.clipWidth = 9,
		.width = 8,
	},
	{ // K
		.clipLeft = 4,
		.clipWidth = 9,
		.width = 8,
	},
	{ // L
		.clipLeft = 4,
		.clipWidth = 8,
		.width = 7,
	},
	{ // M
		.clipLeft = 2,
		.clipWidth = 11,
		.width = 10,
	},
	{ // N
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // O
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // P
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // Q
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // R
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // S
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // T
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // U
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // V
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // W
		.clipLeft = 2,
		.clipWidth = 11,
		.width = 10,
	},
	{ // X
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // Y
		.clipLeft = 3,
		.clipWidth = 10,
		.width = 9,
	},
	{ // Z
		.clipLeft = 4,
		.clipWidth = 8,
		.width = 7,
	},
};

EWRAM_DATA const Metrics largeFontMetrics = {
	.glyphs = largeFontGlyphs
};

EWRAM_DATA const Glyph thinFontGlyphs[128] = {
	{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
	{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
	{ .width = 3 }, {}, {}, {}, {}, {}, {}, {}, {}, {},
	{ // *
		.clipLeft = 0,
		.clipWidth = 7,
		.width = 6
	}, {}, {}, {}, {}, {},
	{ // 0
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // 1
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // 2
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // 3
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // 4
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // 5
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // 6
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // 7
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // 8
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // 9
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // :
		.clipLeft = 1,
		.clipWidth = 10,
		.width = 9,
	},
	{ // ;
		.clipLeft = 1,
		.clipWidth = 10,
		.width = 9,
	},
	{ // <
		.clipLeft = 1,
		.clipWidth = 10,
		.width = 9,
	},
	{ // =
		.clipLeft = 1,
		.clipWidth = 10,
		.width = 9,
	},
	{ // >
		.clipLeft = 1,
		.clipWidth = 10,
		.width = 9,
	},
	{ // ?
		.clipLeft = 1,
		.clipWidth = 10,
		.width = 9,
	},
	{ // @
		.clipLeft = 1,
		.clipWidth = 10,
		.width = 9,
	},
	{ // A
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // B
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // C
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // D
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // E
		.clipLeft = 1,
		.clipWidth = 5,
		.width = 4,
	},
	{ // F
		.clipLeft = 1,
		.clipWidth = 5,
		.width = 8,
	},
	{ // G
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // H
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // I
		.clipLeft = 5,
		.clipWidth = 6,
		.width = 5,
	},
	{ // J
		.clipLeft = 4,
		.clipWidth = 5,
		.width = 8,
	},
	{ // K
		.clipLeft = 4,
		.clipWidth = 5,
		.width = 8,
	},
	{ // L
		.clipLeft = 1,
		.clipWidth = 5,
		.width = 4,
	},
	{ // M
		.clipLeft = 0,
		.clipWidth = 7,
		.width = 6,
	},
	{ // N
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // O
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // P
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // Q
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // R
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // S
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // T
		.clipLeft = 0,
		.clipWidth = 7,
		.width = 6,
	},
	{ // U
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // V
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // W
		.clipLeft = 0,
		.clipWidth = 11,
		.width = 6,
	},
	{ // X
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // Y
		.clipLeft = 1,
		.clipWidth = 6,
		.width = 5,
	},
	{ // Z
		.clipLeft = 4,
		.clipWidth = 8,
		.width = 7,
	},
};

EWRAM_DATA const Metrics thinFontMetrics = {
	.glyphs = thinFontGlyphs
};

EWRAM_DATA const Font largeFont = {
	.gridW = 16,
	.gridH = 16,
	.baseline = 0,
	.grid = (u16*) large_font,
	.metrics = &largeFontMetrics
};

EWRAM_DATA const Font thinFont = {
	.gridW = 8,
	.gridH = 16,
	.baseline = 0,
	.grid = (u16*) thin_font,
	.metrics = &thinFontMetrics
};

IWRAM_CODE void renderText(const char* text, const Textarea* destination, const Font* font) {
	int x, y, i;
	int lastX = 0;
	u16* pixels = destination->destination;
	size_t len = strlen(text);
	switch (destination->align) {
	case TextLeft:
	default:
		x = destination->clipX;
		break;
	case TextCenter:
		x = 0;
		for (i = 0; i < len; ++i) {
			x += font->metrics->glyphs[(int) text[i]].width;
		}
		x = destination->clipX + ((destination->clipW - x) >> 1);
		break;
	case TextRight:
		x = 0;
		for (i = 0; i < len; ++i) {
			x += font->metrics->glyphs[(int) text[i]].width;
		}
		x = destination->clipX + (destination->clipW - x);
		break;
	}
	for (i = 0; i < len; ++i) {
		int startX = x;
		for (y = 0; y < destination->clipH; ++y) {
			int innerY = destination->clipY + y;
			for (x = 0; x < font->metrics->glyphs[(int) text[i]].clipWidth; ++x) {
				int innerX = font->metrics->glyphs[(int) text[i]].clipLeft + x;
				int gridPos = (text[i] >> 4) * (font->gridH * font->gridW * 16 >> 1); // Row
				gridPos += (text[i] & 0xF) * (font->gridW >> 1);
				u16 glyphPixelBank = font->grid[(gridPos >> 1) + (innerX >> 2) + (y * font->gridW * 16 >> 2)];
				int glyphPixel = (glyphPixelBank >> (4 * (1 ^ (innerX & 3)))) & 0xF;

				innerX = startX + x;
				int swizzled = (innerY & 0x7) * 4; // Row
				swizzled += (innerX & 0x7) >> 1; // Column
				swizzled += (innerX >> 3) * 0x20 + (innerY >> 3) * 0x400; // Cell
				if (innerX >= lastX) {
					pixels[swizzled >> 1] &= ~(0xF << (4 * (innerX & 3)));
				}
				pixels[swizzled >> 1] |= glyphPixel << (4 * (innerX & 3));
			}
		}
		lastX = startX + font->metrics->glyphs[(int) text[i]].clipWidth;
		x = startX + font->metrics->glyphs[(int) text[i]].width;
	}
}

void mapText(u16* mapData, int startX, int endX, int startY, int endY, int palette) {
	int x, y;
	for (y = startY; y < endY; ++y) {
		for (x = startX; x < endX; ++x) {
			mapData[x + y * 32] = (x + y * 32) | (palette << 12);
		}
	}
}

void remapText(u16* mapData, int oldX, int oldY, int startX, int endX, int startY, int endY, int palette) {
	int x, y;
	int diffX = startX - oldX;
	int diffY = startY - oldY;
	for (y = startY; y < endY; ++y) {
		for (x = startX; x < endX; ++x) {
			mapData[x + y * 32] = (x - diffX + (y - diffY) * 32) | (palette << 12);
		}
	}
}

void unmapText(u16* mapData, int startX, int endX, int startY, int endY) {
	int x, y;
	for (y = startY; y < endY; ++y) {
		for (x = startX; x < endX; ++x) {
			mapData[x + y * 32] = 0;
		}
	}
}