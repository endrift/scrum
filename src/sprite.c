#include "sprite.h"

#include <gba_dma.h>
#include <gba_sprites.h>

static union {
	struct {
		Sprite attr;
		u16 : 16;
	} obj[128];

	struct {
		u16 : 16;
		u16 : 16;
		u16 : 16;
		u16 a;
		u16 : 16;
		u16 : 16;
		u16 : 16;
		u16 b;
		u16 : 16;
		u16 : 16;
		u16 : 16;
		u16 c;
		u16 : 16;
		u16 : 16;
		u16 : 16;
		u16 d;
	} matrix[32];
} __attribute__((packed, aligned(4))) spriteTable;

static int numActiveSprites = 0;

void clearSpriteTable(void) {
	int i;
	for (i = 0; i < 128; ++i) {
		spriteTable.obj[i].attr.raw.a = 0x0200;
	}
	numActiveSprites = 0;
}

void writeSpriteTable(void) {
	DMA0COPY(&spriteTable, OAM, DMA16 | DMA_IMMEDIATE | 512);
}

void updateSprite(const Sprite* sprite, int at) {
	spriteTable.obj[at].attr = *sprite;
}

void insertSprite(const Sprite* sprite, int at) {
	if (numActiveSprites == 128) {
		return;
	}
	int i;
	for (i = numActiveSprites; i > at; --i) {
		spriteTable.obj[i].attr = spriteTable.obj[i - 1].attr;
	}
	spriteTable.obj[at].attr = *sprite;
	++numActiveSprites;
}