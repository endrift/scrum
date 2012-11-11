#include "text.h"

#include <string.h>

#include "fontLarge_bin.h"


u16 textPalLight[4] = { 0x7C1F, 0x7FFF, 0x35E0, 0x1D08 };
unsigned int textPalLightLen = 8;

struct Font {
	unsigned int gridW;
	unsigned int gridH;
	int baseline;
	const u16* grid;
	const Metrics* metrics;
};

const Glyph largeFontGlyphs[128] = {
	{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
	{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
	{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
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

const Metrics largeFontMetrics = {
	.glyphs = largeFontGlyphs
};

const Font largeFont = {
	.gridW = 16,
	.gridH = 16,
	.baseline = 0,
	.grid = (u16*) fontLarge_bin,
	.metrics = &largeFontMetrics
};

__attribute__((section(".iwram"), long_call))
void renderText(const char* text, const Textarea* destination, const Font* font) {
	int x = destination->clipX, y, i;
	u16* pixels = destination->destination;
	size_t len = strlen(text);
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
				pixels[swizzled >> 1] &= ~(0xF << (4 * (innerX & 3)));
				pixels[swizzled >> 1] |= glyphPixel << (4 * (innerX & 3));
			}
		}
		x = startX + font->metrics->glyphs[(int) text[i]].width;
	}
}