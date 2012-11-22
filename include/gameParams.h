#ifndef GAME_PARAMS_H
#define GAME_PARAMS_H

typedef struct GameParameters {
	int dropTimerLength;
	int dropTimerMin;
	int hasFuncs;
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

extern const GameParameters defaultParams;

#endif