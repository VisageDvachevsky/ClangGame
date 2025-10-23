#include <math.h>
#include "game.h"

double optimal_attempts(int max_value) {
    if (max_value < 2) return 1.0;
    double v = log2((double)max_value);
    double c = ceil(v);
    return c < 1.0 ? 1.0 : c;
}

double progress_ratio(const GameState *st, int max_value) {
    double opt = optimal_attempts(max_value);
    double a = (double)st->attempts;
    if (a <= 0.0) return 0.0;
    double r = a / opt;
    if (r > 1.0) r = 1.0;
    return r;
}
