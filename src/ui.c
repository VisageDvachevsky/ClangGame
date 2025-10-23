#include "ui.h"

#include "terminal.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void ui_show_header(void) {
    terminal_cprintf(C_CYAN, C_BOLD "=== Угадай число ===\n" C_RESET);
}

int ui_menu(void) {
    puts("");
    terminal_cprintf(NULL, C_BOLD "Меню:\n" C_RESET);
    puts("  1) Играть");
    puts("  2) Сложность");
    puts("  3) Статистика");
    puts("  4) Сброс статистики");
    puts("  5) Выход");
    int ch = terminal_read_int_range("Ваш выбор: ", 1, 5);
    return ch;
}

void ui_show_stats(const player_stats_t *stats) {
    terminal_cprintf(C_CYAN, C_BOLD "\n— Статистика игрока —\n" C_RESET);
    printf("Сыграно игр : %u\n", stats->games);
    printf("Побед       : %u\n", stats->wins);
    printf("Рейтинг     : %d\n\n", stats->rating);
}

void ui_stats_reset_message(void) {
    terminal_cprintf(C_YELLOW, "Статистика сброшена.\n");
}

void ui_configure_difficulty(game_config_t *cfg) {
    puts("");
    terminal_cprintf(NULL, C_BOLD "Сложность:\n" C_RESET);
    puts("  1) Лёгкая (1..10)");
    puts("  2) Средняя (1..100)");
    puts("  3) Сложная (1..1000)");
    int d = terminal_read_int_range("Ваш выбор: ", 1, 3);
    game_config_set_difficulty(cfg, d);
    terminal_cprintf(C_GREEN, "Ок! Диапазон теперь 1..%d, попыток рекомендовано: %d.\n",
                     cfg->max_number, cfg->max_attempts);
}

void ui_show_progress(const game_config_t *cfg, const game_state_t *state) {
    int width = 30;
    double ratio = (double)state->attempts / (double)cfg->max_attempts;
    if (ratio > 1.0) ratio = 1.0;
    int filled = (int)round(ratio * width);
    terminal_cprintf(C_DIM, "[");
    for (int i = 0; i < width; ++i) {
        if (i < filled) terminal_cprintf(C_GREEN, "#");
        else fputc('.', stdout);
    }
    terminal_cprintf(C_DIM, "] %d/%d попыток\n", state->attempts, cfg->max_attempts);
}

void ui_show_hint(game_state_t *state, int guess) {
    int range = state->high_bound - state->low_bound + 1;
    int dist = abs(guess - state->target);
    const char *qual;
    if (dist == 0) qual = "точно!";
    else if (dist > range / 2) qual = "холодно";
    else if (dist > range / 4) qual = "прохладно";
    else if (dist > range / 8) qual = "тепло";
    else qual = "горячо";
    if (guess < state->target) terminal_cprintf(C_YELLOW, "Больше! (%s)\n", qual);
    else if (guess > state->target) terminal_cprintf(C_YELLOW, "Меньше! (%s)\n", qual);
    game_state_update_bounds(state, guess);
}

void ui_show_analysis(const game_config_t *cfg, const game_state_t *state, bool won) {
    puts("");
    terminal_cprintf(C_MAGENTA, C_BOLD "— Аналитика —\n" C_RESET);
    if (won) {
        double opt = game_calc_optimality(cfg, state);
        terminal_cprintf(C_GREEN, "Вы угадали за %d попыток. Оптимальность: %.0f%%\n",
                         state->attempts, opt);
    } else {
        terminal_cprintf(C_RED, "Вы сдались. Число было: %d\n", state->target);
    }
    terminal_cprintf(C_DIM, "Диапазон к концу: [%d..%d]\n", state->low_bound, state->high_bound);
    terminal_cprintf(C_DIM, "История: ");
    for (int i = 0; i < state->history_sz; ++i) {
        printf("%d%s", state->history[i], (i + 1 < state->history_sz) ? ", " : "\n");
    }
    if (state->history_sz == 0) printf("(пусто)\n");
}

void ui_notify_rating_change(int old_rating, int new_rating, int delta) {
    const char *color = (delta >= 0) ? C_GREEN : C_RED;
    terminal_cprintf(color, "Рейтинг: %d -> %d (%+d)\n", old_rating, new_rating, delta);
}

void ui_prompt_continue(void) {
    terminal_wait_enter("Нажмите Enter, чтобы продолжить...\n");
}
