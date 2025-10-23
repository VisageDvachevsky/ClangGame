#pragma once

#include <stdbool.h>

void system_seed_rng(void);
void system_setup_signals(void);
bool system_was_interrupted(void);
void system_clear_interrupted(void);
