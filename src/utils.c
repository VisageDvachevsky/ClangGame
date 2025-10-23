#include <stdlib.h>
#include "game.h"

int parse_positive_int(const char *s, int fallback) {
    if (!s || !*s) return fallback;
    char *e = NULL;
    long v = strtol(s, &e, 10);
    if (e && *e == '\0' && v > 0 && v <= 1000000000L) return (int)v;
    return fallback;
}
