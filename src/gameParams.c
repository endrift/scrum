#include "gameParams.h"

const GameParameters defaultParams = {
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

const GameParameters easyParams = {
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

const GameParameters hardParams = {
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