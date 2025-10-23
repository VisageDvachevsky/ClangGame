#include "game.h"
#include "gameplay.h"
#include "stats.h"
#include "system.h"
#include "terminal.h"
#include "ui.h"

int main(void) {
    terminal_enable_ansi_if_needed();
    system_seed_rng();
    system_setup_signals();
    terminal_clear_screen();
    ui_show_header();

    player_stats_t stats;
    stats_load(&stats);

    game_config_t cfg;
    game_config_init(&cfg);

    for (;;) {
        int choice = ui_menu();
        switch (choice) {
            case 1:
                gameplay_play(&stats, &cfg);
                break;
            case 2:
                ui_configure_difficulty(&cfg);
                break;
            case 3:
                ui_show_stats(&stats);
                break;
            case 4:
                stats_reset(&stats);
                ui_stats_reset_message();
                break;
            case 5:
                terminal_cprintf(C_CYAN, "Пока!\n");
                return 0;
        }
    }
}
