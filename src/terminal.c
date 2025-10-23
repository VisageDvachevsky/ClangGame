#include "terminal.h"

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define isatty _isatty
#define STDOUT_FILENO 1
#else
#include <unistd.h>
#endif

static int g_use_color = 1;

void terminal_enable_ansi_if_needed(void) {
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(h, &mode)) {
            mode |= 0x0004;
            SetConsoleMode(h, mode);
        }
    }
#endif
    if (!isatty(STDOUT_FILENO)) g_use_color = 0;
}

void terminal_cprintf(const char *color, const char *fmt, ...) {
    va_list ap;
    if (g_use_color && color) fputs(color, stdout);
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
    if (g_use_color && color) fputs(C_RESET, stdout);
}

void terminal_clear_screen(void) {
    if (!g_use_color) return;
    fputs("\033[H\033[2J", stdout);
}

void terminal_wait_enter(const char *prompt) {
    if (prompt) fputs(prompt, stdout);
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
}

int terminal_read_int_range(const char *prompt, int lo, int hi) {
    for (;;) {
        if (prompt) fputs(prompt, stdout);
        fflush(stdout);
        char buf[128];
        if (!fgets(buf, sizeof(buf), stdin)) return lo;
        char *p = buf;
        while (isspace((unsigned char)*p)) ++p;
        char *end = p + strlen(p);
        while (end > p && isspace((unsigned char)end[-1])) --end;
        *end = '\0';
        char *q = NULL;
        errno = 0;
        long v = strtol(p, &q, 10);
        if (errno == 0 && q && *q == '\0') {
            int iv = (int)v;
            if (iv >= lo && iv <= hi) return iv;
        }
        terminal_cprintf(C_YELLOW, "Введите целое число в диапазоне [%d..%d].\n", lo, hi);
    }
}
