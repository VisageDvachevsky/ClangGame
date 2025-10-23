#pragma once

#include <stdarg.h>

#define C_RESET   "\033[0m"
#define C_BOLD    "\033[1m"
#define C_DIM     "\033[2m"
#define C_RED     "\033[31m"
#define C_GREEN   "\033[32m"
#define C_YELLOW  "\033[33m"
#define C_BLUE    "\033[34m"
#define C_MAGENTA "\033[35m"
#define C_CYAN    "\033[36m"
#define C_WHITE   "\033[37m"

void terminal_enable_ansi_if_needed(void);
void terminal_cprintf(const char *color, const char *fmt, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 2, 3)))
#endif
    ;
void terminal_clear_screen(void);
void terminal_wait_enter(const char *prompt);
int  terminal_read_int_range(const char *prompt, int lo, int hi);
