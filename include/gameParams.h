#ifndef GAME_PARAMS_H
#define GAME_PARAMS_H

#define NUM_GAME_MODES 3

typedef struct GameParameters {
	const char* modeName;
	int dropTimerLength;
	int dropTimerMin;
	int rampSpeed;
	int bugShuntThreshold;
	int bugEntryThreshold;
	int bugKickThreshold;
	int maxBugs;
	int hasColorMismatchBugs;
	int bugSpeed;
	int bugSpeedMax;
	int bulletsMax;
	int bulletCooldown;
} GameParameters;

extern GameParameters currentParams;
extern int gameMode;
extern const GameParameters* modes[];

#endif