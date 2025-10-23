#pragma once

#include <stdbool.h>

#include "game.h"
#include "stats.h"

void ui_show_header(void);
int  ui_menu(void);
void ui_show_stats(const player_stats_t *stats);
void ui_stats_reset_message(void);
void ui_configure_difficulty(game_config_t *cfg);
void ui_show_progress(const game_config_t *cfg, const game_state_t *state);
void ui_show_hint(game_state_t *state, int guess);
void ui_show_analysis(const game_config_t *cfg, const game_state_t *state, bool won);
void ui_notify_rating_change(int old_rating, int new_rating, int delta);
void ui_prompt_continue(void);
