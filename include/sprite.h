#ifndef SPRITE_H
#define SPRITE_H

#include <gba_types.h>

typedef union Sprite {
	struct {
		u16 a;
		u16 b;
		u16 c;
	} raw;

	struct {
		u8 y : 8;
		u8 transformed : 1;
		u8 disable : 1;
		u8 mode : 2;
		u8 mosaic : 1;
		u8 is256color : 1;
		u8 shape : 2;

		u16 x : 9;
		u8 : 3;
		u8 hflip : 1;
		u8 vflip : 1;
		u8 size : 2;

		u16 base : 10;
		u8 priority : 2;
		u8 palette : 4;
	};

	struct {
		u16 : 9;
		u8 doublesize : 1;
		u8 : 6;

		u16 : 9;
		u8 transformGroup : 5;
		u8 : 2;

		u16 : 16;
	};
} __attribute__((packed)) Sprite;

void clearSpriteTable(void);
void writeSpriteTable(void);

int activeSprites(void);
int appendSprite(const Sprite* sprite);
void updateSprite(const Sprite* sprite, int at);
void insertSprite(const Sprite* sprite, int at);
void removeSprite(int at);

void* affineTable(int index);

#endif