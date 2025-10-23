#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct GameConfig GameConfig;
typedef struct GameState  GameState;
typedef struct Stats      Stats;

struct GameConfig {
    int max_value;
    int max_levels;
    int base_hints;
};

struct GameState {
    int level;
    int target;
    int attempts;
    int hints_left;
    bool finished;
};

struct Stats {
    double rating;
    int games_played;
    int wins;
};

void rng_seed(void);
int rng_range(int lo, int hi);

void ui_clear(void);
void ui_title(void);
void ui_print(const char *s);
void ui_printf(const char *fmt, ...);
int  ui_read_int(const char *prompt);
char ui_read_char(const char *prompt);

double optimal_attempts(int max_value);
double progress_ratio(const GameState *st, int max_value);

void stats_init(Stats *stats);
void stats_update(Stats *stats, const GameState *gs, int max_value);
void stats_print(const Stats *stats);

void game_init(GameState *st, const GameConfig *cfg);
void game_new_round(GameState *st, const GameConfig *cfg);
bool game_handle_guess(GameState *st, int guess);
int  game_hint(GameState *st, int max_value);

int parse_positive_int(const char *s, int fallback);
