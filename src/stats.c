#include <stdio.h>
#include <math.h>
#include "game.h"

void stats_init(Stats *stats) {
    stats->rating = 1000.0;
    stats->games_played = 0;
    stats->wins = 0;
}

void stats_update(Stats *stats, const GameState *gs, int max_value) {
    double opt = optimal_attempts(max_value);
    double denom = (double)(gs->attempts <= 0 ? 1 : gs->attempts);
    double eff = opt / denom;
    double delta = 50.0 * eff;
    stats->rating = round(stats->rating + delta);
    stats->games_played += 1;
    stats->wins += 1;
}

void stats_print(const Stats *stats) {
    printf("Игры: %d, Победы: %d, Рейтинг: %.0f\n", stats->games_played, stats->wins, stats->rating);
}
