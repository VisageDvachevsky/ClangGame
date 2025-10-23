#pragma once

#include <stdbool.h>

typedef struct {
    int difficulty;
    int max_number;
    int max_attempts;
} game_config_t;

typedef struct {
    int target;
    int attempts;
    int low_bound;
    int high_bound;
    int *history;
    int history_cap;
    int history_sz;
} game_state_t;

void game_config_init(game_config_t *cfg);
void game_config_set_difficulty(game_config_t *cfg, int difficulty);
int  game_optimal_attempts(int max_number);

void game_state_init(game_state_t *state, const game_config_t *cfg);
void game_state_free(game_state_t *state);
void game_state_record_guess(game_state_t *state, int guess);
void game_state_update_bounds(game_state_t *state, int guess);

double game_calc_optimality(const game_config_t *cfg, const game_state_t *state);
