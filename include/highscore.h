#ifndef HIGHSCORE_H
#define HIGHSCORE_H

#include "runloop.h"

#define NUM_HIGH_SCORES 10

typedef struct Score {
	char name[8];
	unsigned int score;
	unsigned int lines;
} Score;

typedef struct PaddedScore {
	Score score;

	int padding[12];
} PaddedScore;

typedef struct ScoreDB {
	int paddingTop[16];

	PaddedScore scores[NUM_HIGH_SCORES];

	int paddingBottom[240 - (16 * NUM_HIGH_SCORES)];
} ScoreDB;

void initSRAM(void);
const Score* getHighScore(int gameMode, int place);
int isHighScore(int gameMode, const Score* score);
void enterHighScore(int gameMode, const Score* score);
void registerHighScore(int gameMode, const Score* score);

extern Runloop displayHighScores;

#endif