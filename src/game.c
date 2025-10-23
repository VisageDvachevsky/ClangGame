#include <stdlib.h>
#include "game.h"

void game_init(GameState *st, const GameConfig *cfg) {
    st->level = 1;
    st->attempts = 0;
    st->hints_left = cfg->base_hints;
    st->finished = false;
    st->target = rng_range(1, cfg->max_value);
}

void game_new_round(GameState *st, const GameConfig *cfg) {
    st->level += 1;
    st->attempts = 0;
    st->hints_left = cfg->base_hints - (st->level - 1);
    if (st->hints_left < 0) st->hints_left = 0;
    st->finished = false;
    st->target = rng_range(1, cfg->max_value);
}

bool game_handle_guess(GameState *st, int guess) {
    st->attempts += 1;
    if (guess == st->target) {
        st->finished = true;
        return true;
    }
    return false;
}

int game_hint(GameState *st, int max_value) {
    if (st->hints_left <= 0) return -1;
    st->hints_left -= 1;
    int d1 = st->target - 1;
    int d2 = max_value - st->target;
    int d = d1 < d2 ? d1 : d2;
    if (d < 0) d = -d;
    return d;
}
