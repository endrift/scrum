#include "gameParams.h"

const GameParameters defaultParams = {
	.dropTimerLength = 300,
	.dropTimerMin = 150,
	.hasFuncs = 1,
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

const GameParameters easyParams = {
	.dropTimerLength = 360,
	.dropTimerMin = 180,
	.hasFuncs = 1,
	.bugShuntThreshold = 25,
	.bugEntryThreshold = 1,
	.bugKickThreshold = 0,
	.maxBugs = 50,
	.hasColorMismatchBugs = 1,
	.bugSpeed = 100,
	.bugSpeedMax = 200,
	.bulletsMax = 3,
	.bulletCooldown = 15
};

const GameParameters hardParams = {
	.dropTimerLength = 200,
	.dropTimerMin = 100,
	.hasFuncs = 1,
	.bugShuntThreshold = 15,
	.bugEntryThreshold = 5,
	.bugKickThreshold = 4,
	.maxBugs = 20,
	.hasColorMismatchBugs = 1,
	.bugSpeed = 200,
	.bugSpeedMax = 300,
	.bulletsMax = 1,
	.bulletCooldown = 30
};