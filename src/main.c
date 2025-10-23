#include <stdio.h>
#include <stdlib.h>
#include "game.h"

int main(void) {
    rng_seed();
    GameConfig cfg = { .max_value = 100, .max_levels = 5, .base_hints = 3 };
    Stats stats; stats_init(&stats);
    GameState st; game_init(&st, &cfg);
    ui_clear(); ui_title();

    for (;;) {
        ui_printf("Уровень %d/%d, диапазон 1..%d\n", st.level, cfg.max_levels, cfg.max_value);
        ui_printf("Оптимум попыток: %.0f\n", optimal_attempts(cfg.max_value));
        for (;;) {
            ui_printf("Подсказки: %d\n", st.hints_left);
            char c = ui_read_char("[g] угадать  [h] подсказка  [q] выход: ");
            if (c == 'q') {
                ui_print("Выход.\n");
                stats_print(&stats);
                return 0;
            } else if (c == 'h') {
                if (st.hints_left <= 0) {
                    ui_print("Подсказок нет.\n");
                } else {
                    int delta = game_hint(&st, cfg.max_value);
                    ui_printf("Подсказка: разница по модулю %d\n", delta);
                }
            } else if (c == 'g') {
                int guess = ui_read_int("Ваш ответ: ");
                if (game_handle_guess(&st, guess)) {
                    ui_print("Верно!\n");
                    stats_update(&stats, &st, cfg.max_value);
                    if (st.level >= cfg.max_levels) {
                        ui_print("Все уровни пройдены.\n");
                        stats_print(&stats);
                        return 0;
                    }
                    game_new_round(&st, &cfg);
                    break;
                } else {
                    if (guess < st.target) ui_print("Больше.\n");
                    else ui_print("Меньше.\n");
                }
            }
            double pr = progress_ratio(&st, cfg.max_value);
            int p = (int)(pr * 100.0 + 0.5);
            ui_printf("Прогресс: %d%%\n", p);
        }
    }
}
