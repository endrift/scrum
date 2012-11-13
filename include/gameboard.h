#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <gba_types.h>

#include "runloop.h"

#define GAMEBOARD_ROWS 16
#define GAMEBOARD_COLS 16
#define GAMEBOARD_DEADZONE 4

void gameBoardInit(void);
void gameBoardDeinit(void);
void gameBoardFrame(u32 framecount);

void gameBoardSetup(void);

extern Runloop gameBoard;

#endif