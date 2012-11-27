#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include "runloop.h"

#define NUM_HIGH_SCORES 10

typedef struct Score {
	char name[8];
	unsigned int score;
	unsigned int lines;
} Score;

void initSRAM(void);
const Score* getHighScore(int gameMode, int place);
int isHighScore(int gameMode, const Score* score);
void registerHighScore(int gameMode, const Score* score);

extern Runloop displayHighScores;

#endif