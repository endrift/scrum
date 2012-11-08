#include "rng.h"

const u32 coeff = 214013;
const u32 bias = 2531011;

static u32 state;

void srand(u32 seed) {
	state = seed;
}

u32 rand() {
	state = state * coeff + bias;
	return state;
}