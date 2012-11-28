#include "highscore.h"

#include <gba_dma.h>
#include <gba_input.h>
#include <gba_video.h>

#include <string.h>

#include "gameParams.h"
#include "intro.h"
#include "key.h"
#include "rng.h"
#include "text.h"
#include "util.h"

#include "game-backdrop.h"
#include "hud-sprites.h"

const char* savetype = "SRAM_V413"; // Make sure emulators that autodetect know we're faking SRAM
const char* magic = "BPM:GAME";

typedef struct PaddedScore {
	Score score;

	int padding[12];
} PaddedScore;
static PaddedScore bufferScore;

static char initialNames[][8] = {
	"BUSHNELL",
	"DIJKSTRA",
	"MIYAMOTO",
	"MORPHEUS",
	"PAJITNOV",
	"THOMPSON",
	"TORVALDS",
	"SWOZNIAK"
};

static int entered = -1;
static Score enteredScore;
static char enteredName[8];
static int enterCharacter = 0;
static int enteredGameMode;

typedef struct ScoreDB {
	int paddingTop[16];

	PaddedScore scores[NUM_HIGH_SCORES];

	int paddingBottom[240 - (16 * NUM_HIGH_SCORES)];
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
	HIGHSCORE_ENTER,
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

void enterHighScore(int gameMode, const Score* score) {
	if (!score->score) {
		return;
	}
	enteredGameMode = gameMode;
	enteredScore = *score;
	entered = -2;
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

static void drawHighScore(const Score* score, int place, int x, int y) {
	static char placeNum[4] = "00.\0";
	formatNumber(placeNum, 2, place + 1);
	renderText(placeNum, &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = x + 2,
		.clipY = y,
		.clipW = 80,
		.clipH = 16
	}, &largeFont);

	if (!score) {
		renderText("NO RECORD", &(Textarea) {
			.destination = TILE_BASE_ADR(2),
			.clipX = x + 88,
			.clipY = y,
			.clipW = 80,
			.clipH = 16
		}, &largeFont);
	} else {
		static char nameBuffer[9] = {};
		static char scoreBuffer[10] = "000000000\0";
		strncpy(nameBuffer, score->name, 8);
		renderText(nameBuffer,  &(Textarea) {
			.destination = TILE_BASE_ADR(2),
			.clipX = x + 32,
			.clipY = y,
			.clipW = 80,
			.clipH = 16
		}, &largeFont);

		formatNumber(scoreBuffer, 9, score->score);
		renderText(scoreBuffer,  &(Textarea) {
			.destination = TILE_BASE_ADR(2),
			.clipX = x + 124,
			.clipY = y,
			.clipW = 80,
			.clipH = 16
		}, &largeFont);
	}
}

static int page;

static void updatePage(void) {
	remapText(SCREEN_BASE_BLOCK(3), 0, 10 * page + 8, 0, 32, 8, 18, 4);
}

static void drawRectangle(int startX, int startY, int endX, int endY) {
	u16* mapBase = SCREEN_BASE_BLOCK(1);

	mapBase[32 * startY + startX] = 600 | CHAR_PALETTE(4);
	mapBase[32 * endY + startX] = 824 | CHAR_PALETTE(4);
	mapBase[32 * startY + endX] = 600 | 0x400 | CHAR_PALETTE(4);
	mapBase[32 * endY + endX] = 824 | 0x400 | CHAR_PALETTE(4);

	int i;
	for (i = startX + 1; i < endX; ++i) {
		mapBase[32 * startY + i] = 601 | CHAR_PALETTE(4);
		mapBase[32 * endY + i] = 825 | CHAR_PALETTE(4);
	}
	for (i = startY + 1; i < endY; ++i) {
		mapBase[32 * i + startX] = 632 | CHAR_PALETTE(4);
		mapBase[32 * i + endX] = 632 | 0x400 | CHAR_PALETTE(4);
	}

	int x, y;
	for (y = 0; y < (endY - startY) >> 1; ++y) {
		for (x = startX + 1; x < endX; ++x) {
			mapBase[x + (y * 2 + startY + 1) * 32] = 7 | CHAR_PALETTE(5);
			if (y == ((endY - startY) >> 1) - 1) {
				mapBase[x + (y * 2 + startY + 2) * 32] = 7 | CHAR_PALETTE(5);
			} else {
				mapBase[x + (y * 2 + startY + 2) * 32] = 9 | CHAR_PALETTE(5);
			}
		}
	}
}

static void drawCharacter(int k) {
	char letter[2] = "\0\0";
	letter[0] = enteredName[k];
	renderText(letter, &(Textarea) {
		.destination = TILE_BASE_ADR(2),
		.clipX = 41 + k * 12,
		.clipY = 64 + 16 * entered,
		.clipW = 12,
		.clipH = 16,
		.align = TextCenter
	}, &largeFont);
}

static void letterOffset(int increment) {
	char letter = enteredName[enterCharacter];
	if (letter >= 'A' && letter <= 'Z') {
		letter += increment;
		if (letter < 'A' || letter > 'Z') {
			enteredName[enterCharacter] = ' ';
		} else {
			enteredName[enterCharacter] = (letter - 'A' + 26) % 26 + 'A';
		}
	} else if (increment < 0) {
		enteredName[enterCharacter] = 'Z';
	} else {
		enteredName[enterCharacter] = 'A';
	}
	clearBlock(TILE_BASE_ADR(2), 41 + enterCharacter * 12, 64 + 16 * (entered - 5 * page), 12, 16);
	drawCharacter(enterCharacter);
}

static void repeatHandler(KeyContext* context, int keys) {
	(void) (context);
	if (keys & KEY_UP) {
		letterOffset(-1);
	}
	if (keys & KEY_DOWN) {
		letterOffset(1);
	}
}

static KeyContext keyContext = {
	.next = {},
	.active = 0,
	.startDelay = 18,
	.repeatDelay = 6,
	.repeatHandler = repeatHandler
};

void highScoresScreenInit(u32 framecount) {
	switchState(HIGHSCORE_FADE_IN, framecount);

	REG_DISPCNT = BG0_ON | BG1_ON;
	REG_BG0CNT = CHAR_BASE(0) | SCREEN_BASE(1) | 2;
	REG_BG1CNT = CHAR_BASE(2) | SCREEN_BASE(3);
	REG_BLDCNT = 0x00FF;

	DMA3COPY(game_backdropTiles, TILE_BASE_ADR(0) + 64, DMA16 | DMA_IMMEDIATE | (game_backdropTilesLen >> 1));
	DMA3COPY(game_backdropPal, &BG_COLORS[16 * 5], DMA16 | DMA_IMMEDIATE | (game_backdropPalLen >> 1));
	DMA3COPY(hud_spritesTiles, TILE_BASE_ADR(1), DMA16 | DMA_IMMEDIATE | (hud_spritesTilesLen >> 1));
	DMA3COPY(hud_spritesPal, &BG_COLORS[16 * 4], DMA16 | DMA_IMMEDIATE | (hud_spritesPalLen >> 2));

	drawRectangle(2, 1, 27, 6);
	drawRectangle(1, 7, 28, 18);

	page = 0;
}

void highScoresScreenDeinit(void) {
}

void highScoresScreenFrame(u32 framecount) {
	scanKeys();
	u16 keys = keysDown();
	u16 unkeys = keysUp();
	switch (state) {
	case HIGHSCORE_FADE_IN:
		if (framecount == startFrame) {
			mapText(SCREEN_BASE_BLOCK(3), 0, 32, 0, 18, 4);
			int i;
			renderText("HIGH SCORES", &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 25,
				.clipY = 16,
				.clipW = 190,
				.clipH = 16,
				.align = TextCenter
			}, &largeFont);
			renderText(currentParams.modeName, &(Textarea) {
				.destination = TILE_BASE_ADR(2),
				.clipX = 25,
				.clipY = 32,
				.clipW = 190,
				.clipH = 16,
				.align = TextCenter
			}, &largeFont);
			if (entered == -2) {
				if (!enteredName[0]) {
					memcpy(enteredName, initialNames[(rand() >> 16) & 7], 8);
				}
				int j;
				for (i = j = 0; i < 10; ++i) {
					const Score* oldScore = getHighScore(gameMode, j);
					if (entered == -2 && (!oldScore || oldScore->score < enteredScore.score)) {
						drawHighScore(&enteredScore, i, 16, 64 + 16 * i);
						int k;
						char letter[2] = "\0\0";
						for (k = 0; k < 8; ++k) {
							letter[0] = enteredName[k];
							renderText(letter, &(Textarea) {
								.destination = TILE_BASE_ADR(2),
								.clipX = 41 + k * 12,
								.clipY =  64 + 16 * i,
								.clipW = 12,
								.clipH = 16,
								.align = TextCenter
							}, &largeFont);
						}
						page = i >= 5;
						updatePage();
						entered = i;
					} else {
						drawHighScore(oldScore, i, 16, 64 + 16 * i);
						++j;
					}
				}
			} else {
				for (i = 0; i < 10; ++i) {
					drawHighScore(getHighScore(gameMode, i), i, 16, 64 + 16 * i);
				}
			}
		}
		if (framecount - startFrame < 32) {
			REG_BLDY = 0x10 - ((framecount - startFrame) >> 1);
		} else if (entered >= 0) {
			switchState(HIGHSCORE_ENTER, framecount);
		} else {
			switchState(HIGHSCORE_DISPLAY, framecount);
		}
		break;
	case HIGHSCORE_DISPLAY:
		if (keys & (KEY_B | KEY_UP)) {
			if (page != 0) {
				page = 0;
				updatePage();
			}
		}

		if (keys & KEY_A) {
			if (page == 0) {
				page = 1;
				updatePage();
			} else {
				switchState(HIGHSCORE_FADE_OUT, framecount);
			}
		}

		if (keys & KEY_DOWN) {
			if (page == 0) {
				page = 1;
				updatePage();
			}
		}

		if (keys & KEY_START) {
			switchState(HIGHSCORE_FADE_OUT, framecount);
		}
		break;
	case HIGHSCORE_ENTER:
		if (unkeys) {
			stopRepeat(&keyContext, unkeys);
		}
		doRepeat(&keyContext, framecount);

		if (keys & KEY_UP) {
			startRepeat(&keyContext, framecount, KEY_UP);
			letterOffset(-1);
		}
		if (keys & KEY_DOWN) {
			startRepeat(&keyContext, framecount, KEY_DOWN);
			letterOffset(1);
		}

		if (keys & (KEY_LEFT | KEY_B)) {
			stopRepeat(&keyContext, KEY_UP | KEY_DOWN);

			if (enterCharacter > 0) {
				drawCharacter(enterCharacter);
				--enterCharacter;
				if ((framecount & 0x3F) > 0x1F) {
					clearBlock(TILE_BASE_ADR(2), 41 + enterCharacter * 12, 64 + 16 * entered, 12, 16);
				}
			}
		}

		if ((enterCharacter < 7) && (keys & (KEY_RIGHT | KEY_A))) {
			stopRepeat(&keyContext, KEY_UP | KEY_DOWN);
			drawCharacter(enterCharacter);
			++enterCharacter;
			if ((framecount & 0x3F) > 0x1F) {
				clearBlock(TILE_BASE_ADR(2), 41 + enterCharacter * 12, 64 + 16 * entered, 12, 16);
			}
		} else if (keys & (KEY_A | KEY_START)) {
			stopRepeat(&keyContext, KEY_UP | KEY_DOWN);
			strncpy(enteredScore.name, enteredName, 8);
			registerHighScore(enteredGameMode, &enteredScore);
			clearBlock(TILE_BASE_ADR(2), 41, 64 + 16 * entered, 96, 16);
			drawHighScore(&enteredScore, entered, 16, 64 + 16 * entered);
			switchState(HIGHSCORE_DISPLAY, framecount);

			// TODO: why is this necessary?
			page = entered >= 5;
			updatePage();
			entered = -1;
		}
		break;
	case HIGHSCORE_FADE_OUT:
		if (framecount == startFrame) {
			REG_BLDCNT = 0x00BF;
		}
		if (framecount - startFrame <= 32) {
			REG_BLDY = ((framecount - startFrame) >> 1);
		} else {
			setRunloop(&intro);
		}
	}

	if (entered >= 0) {
		if (!(framecount & 0x3F)) {
			drawCharacter(enterCharacter);
		} else if (!(framecount & 0x1F)) {
			clearBlock(TILE_BASE_ADR(2), 41 + enterCharacter * 12, 64 + 16 * (entered - 5 * page), 12, 16);
		}
	}
}