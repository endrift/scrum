#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <gba_types.h>

#include "gameParams.h"
#include "runloop.h"
#include "sprite.h"

#define GAMEBOARD_ROWS 16
#define GAMEBOARD_COLS 16
#define GAMEBOARD_DEADZONE 4

void gameBoardInit(u32 framecount);
void gameBoardDeinit(void);
void gameBoardFrame(u32 framecount);

void gameBoardSetup(u32 framecount);
void updateScore(void);
void updateBugFlashing(int type);

inline int ramp(int a, int b);

extern Runloop gameBoard;

typedef struct Row {
	u8 color[GAMEBOARD_COLS + GAMEBOARD_DEADZONE];
	u8 width;
} Row;

typedef struct Block {
	Sprite spriteL;
	Sprite spriteR;
	int indexL;
	int indexR;
	int width;
	int color;
} Block;

typedef struct GameBoard {
	Row rows[GAMEBOARD_ROWS];

	Block active;
	Block next;
	int activeY;
	int timer;

	unsigned int score;
	int lines;
	int branch;
	int bugs;

	int rowsRemaining;

	int difficultyRamp;
} GameBoard;

extern GameBoard* board;

#endif