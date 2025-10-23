#define _POSIX_C_SOURCE 200809L
#include "system.h"

#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static volatile sig_atomic_t g_interrupted = 0;

static void on_signal(int sig) {
    if (sig == SIGINT) g_interrupted = 1;
}

void system_seed_rng(void) {
    const char *env = getenv("GUESS_SEED");
    if (env && *env) {
        unsigned long s = strtoul(env, NULL, 10);
        srand((unsigned)s);
    } else {
        srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&env);
    }
}

void system_setup_signals(void) {
#ifdef _WIN32
    signal(SIGINT, on_signal);
#else
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
#endif
}

bool system_was_interrupted(void) {
    return g_interrupted != 0;
}

void system_clear_interrupted(void) {
    g_interrupted = 0;
}
