#include "text.h"

#include <string.h>

#include "fontLarge_bin.h"

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
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
	{
		.clipLeft = 0,
		.clipWidth = 16,
		.width = 16,
	},
};

const Metrics largeFontMetrics = {
	.glyphs = largeFontGlyphs
};

const Font largeFont = {
	.gridW = 16,
	.gridH = 16,
	.baseline = 3,
	.grid = (u16*) fontLarge_bin,
	.metrics = &largeFontMetrics
};

void renderText(const char* text, const Textarea* destination, const Font* font) {
	int x = destination->clipX, y, i;
	int clearedX = destination->clipX;
	u16* pixels = destination->destination;
	size_t len = strlen(text);
	for (i = 0; i < len; ++i) {
		int startX = x;
		for (y = destination->clipY; y < destination->clipY + destination->clipH; ++y) {
			for (x = 0; x < font->metrics->glyphs[(int) text[i]].clipWidth; ++x) {
				int innerX = font->metrics->glyphs[(int) text[i]].clipLeft + x;
				int gridPos = (text[i] >> 4) * (font->gridH * font->gridW * 16 >> 1); // Row
				gridPos += (text[i] & 0xF) * (font->gridW >> 1);
				u16 glyphPixelBank = font->grid[(gridPos >> 1) + (innerX >> 2) + (y * font->gridW * 16 >> 2)];
				int glyphPixel = (glyphPixelBank >> ((x & 3) * 4)) & 0xF;
				//if (!glyphPixel) {
					//if (x > clearedX) {
					//	++clearedX;
					//} else {
					//	glyphPixel = (pixels[(x >> 2) + y * destination->stride] >> (x & 3)) & 0xF;
					//}
				//}
				innerX += startX;
				int swizzled = (y & 0x7) * 4; // Row
				swizzled += (innerX & 0x7) >> 1; // Column
				swizzled += (innerX >> 3) * 0x20 + (y >> 3) * 0x400; // Cell
				pixels[swizzled >> 1] |= glyphPixel << (((innerX & 3) ^ 1) * 4);
			}
		}
		x = startX + font->metrics->glyphs[(int) text[i]].width;
	}
}