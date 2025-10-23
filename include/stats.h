#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "game.h"

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t games;
    uint32_t wins;
    int32_t rating;
} player_stats_t;

void stats_load(player_stats_t *stats);
void stats_save(const player_stats_t *stats);
void stats_reset(player_stats_t *stats);
void stats_record_game(player_stats_t *stats, bool won);
int  stats_update_rating(player_stats_t *stats, const game_config_t *cfg, const game_state_t *state, bool won);
