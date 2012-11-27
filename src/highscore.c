#include "highscore.h"

#include <gba_dma.h>
#include <gba_video.h>

#include <string.h>

#include "gameParams.h"
#include "text.h"
#include "util.h"

#include "hud-sprites.h"

const char* savetype = "SRAM_V413"; // Make sure emulators that autodetect know we're faking SRAM
const char* magic = "BPM:GAME";

typedef struct PaddedScore {
	Score score;

	int padding[12];
} PaddedScore;
static PaddedScore bufferScore;

typedef struct ScoreDB {
	int paddingTop[16];

	PaddedScore scores[NUM_HIGH_SCORES];

	int paddingBottom[256 - (16 * NUM_HIGH_SCORES)];
} ScoreDB;

typedef struct SRAMBase {
	char magic[8];
	int paddingTop[254];
	ScoreDB scoreDB[NUM_GAME_MODES];
} SRAMBase;

SRAMBase* const sram = (SRAMBase*) 0x0E000000;

void highScoresScreenInit(u32 framecount);
void highScoresScreenDeinit(void);
void highScoresScreenFrame(u32 framecount);

// TODO: refactor this
static enum {
	HIGHSCORE_FADE_IN,
	HIGHSCORE_DISPLAY,
	HIGHSCORE_FADE_OUT
} state;
static u32 startFrame;

static void switchState(int nextState, u32 framecount) {
	state = nextState;
	startFrame = framecount;
	displayHighScores.frame(framecount);
}

Runloop displayHighScores = {
	.init = highScoresScreenInit,
	.deinit = highScoresScreenDeinit,
	.frame = highScoresScreenFrame
};

void initSRAM(void) {
	static char buffer[8];
	byteCopy(buffer, sram->magic, 8);
	if (memcmp(buffer, magic, 8)) {
		byteZero(sram, 0x8000);
		byteCopy(sram->magic, magic, 8);
	}
}

const Score* getHighScore(int gameMode, int place) {
	if (place >= 10 || gameMode >= NUM_GAME_MODES) {
		return 0;
	}

	byteCopy(&bufferScore, &sram->scoreDB[gameMode].scores[place], sizeof(bufferScore));
	if (bufferScore.score.name[0]) {
		return &bufferScore.score;
	}
	return 0;
}

int isHighScore(int gameMode, const Score* score) {
	if (gameMode >= NUM_GAME_MODES) {
		return 0;
	}
	byteCopy(&bufferScore, &sram->scoreDB[gameMode].scores[NUM_HIGH_SCORES - 1], sizeof(bufferScore));
	if (bufferScore.score.name[0]) {
		return score->score > bufferScore.score.score;
	} else {
		return 1;
	}
}

void registerHighScore(int gameMode, const Score* score) {
	if (gameMode >= NUM_GAME_MODES) {
		return;
	}
	PaddedScore* scores = sram->scoreDB[gameMode].scores;
	int i, j;
	for (i = 0; i < NUM_HIGH_SCORES; ++i) {
		byteCopy(&bufferScore, &scores[i], sizeof(bufferScore));
		if (score->score > bufferScore.score.score) {
			for (j = NUM_HIGH_SCORES - 1; j > i; --j) {
				byteCopy(&scores[j], &scores[j - 1], sizeof(PaddedScore));
			}
			bufferScore.score = *score;
			byteCopy(&scores[i], &bufferScore, sizeof(bufferScore));
			break;
		}
	}
}

static void drawHighScore(int gameMode, int place, int x, int y) {
	const Score* score = getHighScore(gameMode, place);
	static char placeNum[4] = "00.\0";
	formatNumber(placeNum, 2, place + 1);
	renderText(placeNum, &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = x,
		.clipY = y,
		.clipW = 80,
		.clipH = 16,
		.baseline = 0
	}, &largeFont);

	if (!score) {
		renderText("NO RECORD", &(Textarea) {
			.destination = TILE_BASE_ADR(2),
			.clipX = x + 88,
			.clipY = y,
			.clipW = 80,
			.clipH = 16,
			.baseline = 0
		}, &largeFont);
	} else {
		static char nameBuffer[9] = {};
		static char scoreBuffer[10] = "000000000\0";
		strncpy(nameBuffer, score->name, 8);
		renderText(nameBuffer,  &(Textarea) {
			.destination = TILE_BASE_ADR(2),
			.clipX = x + 40,
			.clipY = y,
			.clipW = 80,
			.clipH = 16,
			.baseline = 0
		}, &largeFont);

		formatNumber(scoreBuffer, 9, score->score);
		renderText(scoreBuffer,  &(Textarea) {
			.destination = TILE_BASE_ADR(2),
			.clipX = x + 128,
			.clipY = y,
			.clipW = 80,
			.clipH = 16,
			.baseline = 0
		}, &largeFont);
	}
}

void highScoresScreenInit(u32 framecount) {
	switchState(HIGHSCORE_FADE_IN, framecount);

	REG_DISPCNT = BG0_ON | BG1_ON;
	REG_BG0CNT = CHAR_BASE(0) | SCREEN_BASE(1) | 3;
	REG_BG1CNT = CHAR_BASE(2) | SCREEN_BASE(3);
	REG_BLDCNT = 0x00FF;

	DMA3COPY(hud_spritesTiles, TILE_BASE_ADR(1), DMA16 | DMA_IMMEDIATE | (hud_spritesTilesLen >> 1));
	DMA3COPY(hud_spritesPal, &BG_COLORS[16 * 4], DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 2));
}

void highScoresScreenDeinit(void) {
}

void highScoresScreenFrame(u32 framecount) {
	switch (state) {
	case HIGHSCORE_FADE_IN:
		if (framecount == startFrame) {
			mapText(SCREEN_BASE_BLOCK(3), 0, 32, 0, 20, 4);
			int i;
			renderText("HIGH SCORES", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 24,
				.clipY = 16,
				.clipW = 64,
				.clipH = 16,
				.baseline = 0
			}, &largeFont);
			renderText(currentParams.modeName, &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 24,
				.clipY = 32,
				.clipW = 64,
				.clipH = 16,
				.baseline = 0
			}, &largeFont);
			for (i = 0; i < 5; ++i) {
				drawHighScore(gameMode, i, 16, 64 + 16 * i);
			}
		}
		if (framecount - startFrame < 64) {
			REG_BLDY = 0x10 - ((framecount - startFrame) >> 2);
		}
		break;
	case HIGHSCORE_DISPLAY:
		break;
	}
}