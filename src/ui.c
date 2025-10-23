#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "game.h"

void ui_clear(void) {
    printf("\033[2J\033[H");
}

void ui_title(void) {
    printf("ClangGame\n");
}

void ui_print(const char *s) {
    fputs(s, stdout);
}

void ui_printf(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

static void read_line(char *buf, size_t n) {
    if (!fgets(buf, (int)n, stdin)) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len && buf[len - 1] == '\n') buf[len - 1] = '\0';
}

int ui_read_int(const char *prompt) {
    char buf[64];
    for (;;) {
        ui_printf("%s", prompt);
        read_line(buf, sizeof buf);
        char *end = NULL;
        long v = strtol(buf, &end, 10);
        if (end && *end == '\0') return (int)v;
        ui_print("Неверный ввод.\n");
    }
}

char ui_read_char(const char *prompt) {
    char buf[64];
    ui_printf("%s", prompt);
    read_line(buf, sizeof buf);
    for (size_t i = 0; buf[i]; ++i) buf[i] = (char)tolower((unsigned char)buf[i]);
    if (buf[0] == '\0') return 'g';
    return buf[0];
}
