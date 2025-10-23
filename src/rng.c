#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "game.h"

void rng_seed(void) {
    unsigned s = (unsigned)time(NULL);
    s ^= (unsigned)(uintptr_t)&s;
    srand(s);
}

int rng_range(int lo, int hi) {
    int span = hi - lo + 1;
    int r = rand() % span;
    return lo + r;
}
