#include "gameplay.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "system.h"
#include "terminal.h"
#include "ui.h"

void gameplay_play(player_stats_t *stats, const game_config_t *cfg) {
    if (!stats || !cfg) return;
    game_state_t state;
    game_state_init(&state, cfg);

    const char *dbg = getenv("DEBUG_SHOW_TARGET");
    if (dbg && *dbg == '1') {
        terminal_cprintf(C_DIM, "[debug] target=%d\n", state.target);
    }

    terminal_cprintf(C_BOLD, "\nЗагадываю число от 1 до %d. Попробуйте угадать!\n",
                     cfg->max_number);

    bool won = false;
    while (!won) {
        if (system_was_interrupted()) {
            terminal_cprintf(C_RED, "\n^C Понял. Прервём игру аккуратно.\n");
            system_clear_interrupted();
            break;
        }
        ui_show_progress(cfg, &state);
        printf("Ваш ответ (или 0 — сдаться): ");
        fflush(stdout);
        int guess = terminal_read_int_range(NULL, 0, cfg->max_number);
        if (guess == 0) break;
        game_state_record_guess(&state, guess);
        if (guess == state.target) {
            terminal_cprintf(C_GREEN, C_BOLD "Верно! Поздравляю!\n" C_RESET);
            won = true;
            break;
        }
        ui_show_hint(&state, guess);
    }

    ui_show_analysis(cfg, &state, won);
    stats_record_game(stats, won);
    int old_rating = stats->rating;
    int delta = stats_update_rating(stats, cfg, &state, won);
    ui_notify_rating_change(old_rating, stats->rating, delta);
    stats_save(stats);
    game_state_free(&state);

    puts("");
    ui_prompt_continue();
}
