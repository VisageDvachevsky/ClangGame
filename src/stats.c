#include "stats.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint32_t STATS_MAGIC = 0x47554553u;
static const uint32_t STATS_VER   = 1u;

static const char *stats_path(void) {
    static char path[512];
    const char *home = getenv("HOME");
#ifdef _WIN32
    if (!home) home = getenv("USERPROFILE");
#endif
    if (!home) home = ".";
    snprintf(path, sizeof(path), "%s/%s", home, ".guess_stats.bin");
    return path;
}

static void stats_default(player_stats_t *ps) {
    if (!ps) return;
    memset(ps, 0, sizeof(*ps));
    ps->magic = STATS_MAGIC;
    ps->version = STATS_VER;
    ps->rating = 1000;
}

static int clampi(int x, int lo, int hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void stats_load(player_stats_t *stats) {
    if (!stats) return;
    FILE *f = fopen(stats_path(), "rb");
    if (!f) {
        stats_default(stats);
        return;
    }
    player_stats_t tmp;
    size_t n = fread(&tmp, 1, sizeof(tmp), f);
    fclose(f);
    if (n != sizeof(tmp) || tmp.magic != STATS_MAGIC) {
        stats_default(stats);
        return;
    }
    *stats = tmp;
}

void stats_save(const player_stats_t *stats) {
    if (!stats) return;
    FILE *f = fopen(stats_path(), "wb");
    if (!f) return;
    fwrite(stats, 1, sizeof(*stats), f);
    fclose(f);
}

void stats_reset(player_stats_t *stats) {
    if (!stats) return;
    stats_default(stats);
    stats_save(stats);
}

void stats_record_game(player_stats_t *stats, bool won) {
    if (!stats) return;
    stats->games++;
    if (won) stats->wins++;
}

int stats_update_rating(player_stats_t *stats, const game_config_t *cfg, const game_state_t *state, bool won) {
    if (!stats || !cfg || !state) return 0;
    static const int mult[4] = {0, 1, 2, 3};
    int base = 50 * mult[cfg->difficulty];
    int delta = 0;
    if (won) {
        double opt = game_calc_optimality(cfg, state) / 100.0;
        if (opt > 1.5) opt = 1.5;
        delta = (int)lround(base * (0.5 + opt));
    } else {
        delta = -(base / 2);
    }
    int old = stats->rating;
    stats->rating = clampi(stats->rating + delta, 500, 3000);
    return stats->rating - old;
}
