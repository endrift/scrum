#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <gba_types.h>

#include "runloop.h"

#define GAMEBOARD_ROWS 16
#define GAMEBOARD_COLS 16

typedef struct GameBoard {
	struct Row {
		u8 color[GAMEBOARD_COLS];
		u8 width;
	} rows[GAMEBOARD_ROWS];
} GameBoard;

void gameBoardInit(void);
void gameBoardDeinit(void);
void gameBoardFrame(u32 framecount);

Runloop gameBoard;

#endif