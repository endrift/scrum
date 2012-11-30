#include "save.h"

#include <string.h>

#include "gameboard.h"
#include "gameParams.h"
#include "highscore.h"
#include "util.h"

const char* savetype = "SRAM_V413"; // Make sure emulators that autodetect know we're faking SRAM
const char* magic = "BPM:GAME";

typedef struct GameState {
	int isGameSaved : 1;
	int activeBoard : 1;
	int inMinigame : 1;
	int : 29;
	int gameMode;
	int paddingTop[6];
	GameBoard masterBoard;
	int masterBoardPadding[6];
	GameBoard localBoard;
	int localBoardPadding[6];
} GameState;

typedef struct SRAMBase {
	char magic[8];
	int paddingTop[6];
	GameState savedGame;
	int paddingMid[16];
	ScoreDB scoreDB[NUM_GAME_MODES];
} SRAMBase;

SRAMBase* const sram = (SRAMBase*) 0x0E000000;
static GameState saveBuffer;

void initSRAM(void) {
	static char buffer[8];
	byteCopy(buffer, sram->magic, 8);
	if (memcmp(buffer, magic, 8)) {
		byteZero(sram, 0x8000);
		byteCopy(sram->magic, magic, 8);
	}
}

void* scoreBase(void) {
	return sram->scoreDB;
}

int isSavedGame(void) {
	byteCopy(&saveBuffer, &sram->savedGame, 4);
	return saveBuffer.isGameSaved;
}

void saveGame(GameBoard* masterBoard, GameBoard* localBoard) {
	saveBuffer.isGameSaved = 1;
	saveBuffer.activeBoard = board == localBoard;
	saveBuffer.inMinigame = 0;
	saveBuffer.gameMode = gameMode;
	saveBuffer.masterBoard = *masterBoard;
	saveBuffer.localBoard = *localBoard;
	byteCopy(&sram->savedGame, &saveBuffer, sizeof(saveBuffer));
}

void loadGame(GameBoard* masterBoard, GameBoard* localBoard) {
	byteCopy(&saveBuffer, &sram->savedGame, sizeof(saveBuffer));
	if (!saveBuffer.isGameSaved) {
		return;
	}
	board = saveBuffer.activeBoard ? localBoard : masterBoard;
	gameMode = saveBuffer.gameMode;
	currentParams = *modes[gameMode];
	*masterBoard = saveBuffer.masterBoard;
	*localBoard = saveBuffer.localBoard;

	char zero = 0;
	byteCopy(&sram->savedGame, &zero, 1);
}