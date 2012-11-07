#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <gba_types.h>

#include "runloop.h"

typedef struct GameBoard {
	struct Row {
		u8 color[16];
		u8 width;
	} rows[16];
} GameBoard;

void gameBoardInit(void);
void gameBoardDeinit(void);
void gameBoardFrame(u32 framecount);

extern Runloop gameBoard;

#endif