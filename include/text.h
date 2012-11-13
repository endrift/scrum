#ifndef TEXT_H
#define TEXT_H

#include <gba_types.h>

typedef struct Font Font;

typedef struct Textarea {
	void* destination;
	int clipX;
	int clipY;
	unsigned int clipW;
	unsigned int clipH;
	int baseline;
} Textarea;

typedef struct Glyph {
	unsigned int clipLeft;
	unsigned int clipWidth;
	unsigned int width;
} Glyph;

typedef struct Metrics {
	const Glyph* glyphs;
} Metrics;

extern const Font largeFont;
extern const Font thinFont;

extern u16 textPalLight[];
extern unsigned int textPalLightLen;

void renderText(const char* text, const Textarea* destination, const Font* font);
void mapText(u16* mapData, int startX, int endX, int startY, int endY, int palette);

#endif