#ifndef SAVE_H
#define SAVE_H

typedef struct GameBoard GameBoard;

void initSRAM(void);

void* scoreBase(void);

int isSavedGame(void);
void saveGame(GameBoard* masterBoard, GameBoard* localBoard);
void loadGame(GameBoard* masterBoard, GameBoard* localBoard);

#endif