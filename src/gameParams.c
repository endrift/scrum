#include "gameParams.h"

static const GameParameters defaultParams = {
	.modeName = "ENGINEER",
	.dropTimerLength = 300,
	.dropTimerMin = 150,
	.rampSpeed = 4,
	.bugShuntThreshold = 20,
	.bugEntryThreshold = 1,
	.bugKickThreshold = 0,
	.maxBugs = 30,
	.hasColorMismatchBugs = 1,
	.bugSpeed = 150,
	.bugSpeedMax = 250,
	.bulletsMax = 2,
	.bulletCooldown = 20
};

static const GameParameters easyParams = {
	.modeName = "SCRIPT KIDDIE",
	.dropTimerLength = 360,
	.dropTimerMin = 180,
	.rampSpeed = 2,
	.bugShuntThreshold = 25,
	.bugEntryThreshold = 1,
	.bugKickThreshold = 0,
	.maxBugs = 40,
	.hasColorMismatchBugs = 1,
	.bugSpeed = 150,
	.bugSpeedMax = 200,
	.bulletsMax = 2,
	.bulletCooldown = 15
};

static const GameParameters hardParams = {
	.modeName = "SCRUM MASTER",
	.dropTimerLength = 200,
	.dropTimerMin = 100,
	.rampSpeed = 8,
	.bugShuntThreshold = 15,
	.bugEntryThreshold = 5,
	.bugKickThreshold = 0,
	.maxBugs = 20,
	.hasColorMismatchBugs = 1,
	.bugSpeed = 220,
	.bugSpeedMax = 320,
	.bulletsMax = 1,
	.bulletCooldown = 30
};

const GameParameters* modes[] = {
	&easyParams,
	&defaultParams,
	&hardParams,
	0
};

GameParameters currentParams;
int gameMode;