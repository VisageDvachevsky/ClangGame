#include "game.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static int clamp_difficulty(int d) {
    if (d < 1) return 1;
    if (d > 3) return 3;
    return d;
}

void game_config_init(game_config_t *cfg) {
    if (!cfg) return;
    cfg->difficulty = 2;
    game_config_set_difficulty(cfg, cfg->difficulty);
}

void game_config_set_difficulty(game_config_t *cfg, int difficulty) {
    if (!cfg) return;
    cfg->difficulty = clamp_difficulty(difficulty);
    switch (cfg->difficulty) {
        case 1: cfg->max_number = 10; break;
        case 2: cfg->max_number = 100; break;
        default: cfg->difficulty = 3; cfg->max_number = 1000; break;
    }
    cfg->max_attempts = game_optimal_attempts(cfg->max_number) + 1;
}

int game_optimal_attempts(int max_number) {
    if (max_number <= 1) return 1;
    double v = ceil(log2((double)max_number));
    return (int)v;
}

void game_state_init(game_state_t *state, const game_config_t *cfg) {
    if (!state || !cfg) return;
    memset(state, 0, sizeof(*state));
    state->low_bound = 1;
    state->high_bound = cfg->max_number;
    state->target = (rand() % cfg->max_number) + 1;
    state->history_cap = cfg->max_attempts;
    if (state->history_cap > 0) {
        state->history = (int *)calloc((size_t)state->history_cap, sizeof(int));
        if (!state->history) state->history_cap = 0;
    }
}

void game_state_free(game_state_t *state) {
    if (!state) return;
    free(state->history);
    state->history = NULL;
    state->history_cap = 0;
    state->history_sz = 0;
}

void game_state_record_guess(game_state_t *state, int guess) {
    if (!state) return;
    state->attempts++;
    if (state->history && state->history_sz < state->history_cap) {
        state->history[state->history_sz++] = guess;
    }
}

void game_state_update_bounds(game_state_t *state, int guess) {
    if (!state) return;
    if (guess < state->target && guess >= state->low_bound) {
        state->low_bound = guess + 1;
    } else if (guess > state->target && guess <= state->high_bound) {
        state->high_bound = guess - 1;
    }
}

double game_calc_optimality(const game_config_t *cfg, const game_state_t *state) {
    if (!cfg || !state) return 0.0;
    int opt = game_optimal_attempts(cfg->max_number);
    if (state->attempts <= 0) return 0.0;
    double v = (double)opt / (double)state->attempts * 100.0;
    if (v < 0.0) v = 0.0;
    if (v > 200.0) v = 200.0;
    return v;
}
