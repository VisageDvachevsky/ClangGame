#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define isatty _isatty
#define STDOUT_FILENO 1
#else
#include <unistd.h>
#endif

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

static int g_use_color = 1;

static void enable_ansi_if_needed(void) {
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

static void cprintf(const char *color, const char *fmt, ...) {
	va_list ap;
	if (g_use_color && color) fputs(color, stdout);
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	if (g_use_color && color) fputs(C_RESET, stdout);
}

static void seed_rng(void) {
	const char *env = getenv("GUESS_SEED");
	if (env && *env) {
		unsigned long s = strtoul(env, NULL, 10);
		srand((unsigned)s);
	} else {
		srand((unsigned)time(NULL) ^ (unsigned)(uintptr_t)&env);
	}
}

static volatile sig_atomic_t g_interrupted = 0;

static void on_signal(int sig) {
	if (sig == SIGINT) g_interrupted = 1;
}

static void setup_signals(void) {
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

static int clampi(int x, int lo, int hi) {
	if (x < lo) return lo;
	if (x > hi) return hi;
	return x;
}

static void clear_screen(void) {
	if (!g_use_color) return;
	fputs("\033[H\033[2J", stdout);
}

static void wait_enter(const char *prompt) {
	if (prompt) fputs(prompt, stdout);
	int ch;
	while ((ch = getchar()) != '\n' && ch != EOF) {}
}

static int read_int_range(const char *prompt, int lo, int hi) {
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
		cprintf(C_YELLOW, "Введите целое число в диапазоне [%d..%d].\n", lo, hi);
	}
}

typedef struct {
	uint32_t magic;
	uint32_t version;
	uint32_t games;
	uint32_t wins;
	int32_t  rating;
} player_stats_t;

static const uint32_t STATS_MAGIC = 0x47554553u;
static const uint32_t STATS_VER   = 1u;

static const char *stats_path(void) {
	static char path[512];
	const char *home = getenv("HOME");
#ifdef _WIN32
	if (!home) home = getenv("USERPROFILE");
#endif
	if (!home) home = ".";
	snprintf(path, sizeof(path), "%s/%s", home, ".guess_stats.bin");
	return path;
}

static void stats_default(player_stats_t *ps) {
	memset(ps, 0, sizeof(*ps));
	ps->magic = STATS_MAGIC;
	ps->version = STATS_VER;
	ps->rating = 1000;
}

static void stats_load(player_stats_t *ps) {
	FILE *f = fopen(stats_path(), "rb");
	if (!f) { stats_default(ps); return; }
	player_stats_t tmp;
	size_t n = fread(&tmp, 1, sizeof(tmp), f);
	fclose(f);
	if (n != sizeof(tmp) || tmp.magic != STATS_MAGIC) {
		stats_default(ps);
		return;
	}
	*ps = tmp;
}

static void stats_save(const player_stats_t *ps) {
	FILE *f = fopen(stats_path(), "wb");
	if (!f) return;
	fwrite(ps, 1, sizeof(*ps), f);
	fclose(f);
}

typedef struct {
	int difficulty;
	int max_number;
	int max_attempts;
} game_config_t;

typedef struct {
	int target;
	int attempts;
	int low_bound, high_bound;
	int *history;
	int history_cap;
	int history_sz;
} game_state_t;

static int optimal_attempts(int max_number) {
	if (max_number <= 1) return 1;
	double v = ceil(log2((double)max_number));
	return (int)v;
}

static void config_by_difficulty(game_config_t *cfg) {
	switch (cfg->difficulty) {
		case 1: cfg->max_number = 10; break;
		case 2: cfg->max_number = 100; break;
		default: cfg->difficulty = 3; cfg->max_number = 1000; break;
	}
	cfg->max_attempts = optimal_attempts(cfg->max_number) + 1;
}

static void gs_free(game_state_t *gs) {
	free(gs->history);
	gs->history = NULL;
	gs->history_cap = gs->history_sz = 0;
}

static void gs_init(game_state_t *gs, const game_config_t *cfg) {
	memset(gs, 0, sizeof(*gs));
	gs->low_bound = 1;
	gs->high_bound = cfg->max_number;
	gs->target = (rand() % cfg->max_number) + 1;
	gs->attempts = 0;
	gs->history_cap = cfg->max_attempts;
	gs->history = (int*)calloc((size_t)gs->history_cap, sizeof(int));
	gs->history_sz = 0;
}

static void header(void) {
	cprintf(C_CYAN, C_BOLD "=== Угадай число ===\n" C_RESET);
}

static void show_progress(const game_config_t *cfg, const game_state_t *gs) {
	int width = 30;
	double ratio = (double)gs->attempts / (double)cfg->max_attempts;
	if (ratio > 1.0) ratio = 1.0;
	int filled = (int)round(ratio * width);
	cprintf(C_DIM, "[");
	for (int i = 0; i < width; ++i) {
		if (i < filled) cprintf(C_GREEN, "#");
		else fputc('.', stdout);
	}
	cprintf(C_DIM, "] %d/%d попыток\n", gs->attempts, cfg->max_attempts);
}

static void show_hint(game_state_t *gs, int guess) {
	int range = gs->high_bound - gs->low_bound + 1;
	int dist = abs(guess - gs->target);
	const char *qual;
	if (dist == 0) qual = "точно!";
	else if (dist > range / 2) qual = "холодно";
	else if (dist > range / 4) qual = "прохладно";
	else if (dist > range / 8) qual = "тепло";
	else qual = "горячо";
	if (guess < gs->target) cprintf(C_YELLOW, "Больше! (%s)\n", qual);
	else if (guess > gs->target) cprintf(C_YELLOW, "Меньше! (%s)\n", qual);
	if (guess < gs->target && guess >= gs->low_bound) gs->low_bound = guess + 1;
	if (guess > gs->target && guess <= gs->high_bound) gs->high_bound = guess - 1;
}

static double calc_optimality(const game_config_t *cfg, const game_state_t *gs) {
	int opt = optimal_attempts(cfg->max_number);
	if (gs->attempts <= 0) return 0.0;
	double v = (double)opt / (double)gs->attempts * 100.0;
	if (v < 0) v = 0;
	if (v > 200) v = 200;
	return v;
}

static void analyze_and_print(const game_config_t *cfg, const game_state_t *gs, bool won) {
	puts("");
	cprintf(C_MAGENTA, C_BOLD "— Аналитика —\n" C_RESET);
	if (won) {
		double opt = calc_optimality(cfg, gs);
		cprintf(C_GREEN, "Вы угадали за %d попыток. Оптимальность: %.0f%%\n", gs->attempts, opt);
	} else {
		cprintf(C_RED, "Вы сдались. Число было: %d\n", gs->target);
	}
	cprintf(C_DIM, "Диапазон к концу: [%d..%d]\n", gs->low_bound, gs->high_bound);
	cprintf(C_DIM, "История: ");
	for (int i = 0; i < gs->history_sz; ++i) {
		printf("%d%s", gs->history[i], (i + 1 < gs->history_sz) ? ", " : "\n");
	}
}

static void update_rating(player_stats_t *ps, const game_config_t *cfg, const game_state_t *gs, bool won) {
	static const int mult[4] = {0, 1, 2, 3};
	int base = 50 * mult[cfg->difficulty];
	int delta = 0;
	if (won) {
		double opt = calc_optimality(cfg, gs) / 100.0;
		if (opt > 1.5) opt = 1.5;
		delta = (int)round(base * (0.5 + opt));
	} else {
		delta = -(base / 2);
	}
	int old = ps->rating;
	ps->rating = clampi(ps->rating + delta, 500, 3000);
	cprintf(delta >= 0 ? C_GREEN : C_RED, "Рейтинг: %d -> %d (%+d)\n", old, ps->rating, delta);
}

static int menu(void) {
	puts("");
	cprintf(C_BOLD, "Меню:\n");
	puts("  1) Играть");
	puts("  2) Сложность");
	puts("  3) Статистика");
	puts("  4) Сброс статистики");
	puts("  5) Выход");
	int ch = read_int_range("Ваш выбор: ", 1, 5);
	return ch;
}

static void show_stats(const player_stats_t *ps) {
	cprintf(C_CYAN, C_BOLD "\n— Статистика игрока —\n" C_RESET);
	printf("Сыграно игр : %u\n", ps->games);
	printf("Побед       : %u\n", ps->wins);
	printf("Рейтинг     : %d\n", ps->rating);
	puts("");
}

static void reset_stats(player_stats_t *ps) {
	stats_default(ps);
	stats_save(ps);
	cprintf(C_YELLOW, "Статистика сброшена.\n");
}

static void choose_difficulty(game_config_t *cfg) {
	puts("");
	cprintf(C_BOLD, "Сложность:\n");
	puts("  1) Лёгкая (1..10)");
	puts("  2) Средняя (1..100)");
	puts("  3) Сложная (1..1000)");
	int d = read_int_range("Ваш выбор: ", 1, 3);
	cfg->difficulty = d;
	config_by_difficulty(cfg);
	cprintf(C_GREEN, "Ок! Диапазон теперь 1..%d, попыток рекомендовано: %d.\n", cfg->max_number, cfg->max_attempts);
}

static void play_game(player_stats_t *ps, const game_config_t *cfg) {
	game_state_t gs;
	gs_init(&gs, cfg);
	const char *dbg = getenv("DEBUG_SHOW_TARGET");
	if (dbg && *dbg == '1') cprintf(C_DIM, "[debug] target=%d\n", gs.target);
	cprintf(C_BOLD, "\nЗагадываю число от 1 до %d. Попробуйте угадать!\n", cfg->max_number);
	bool won = false;
	while (!won) {
		if (g_interrupted) {
			cprintf(C_RED, "\n^C Понял. Прервём игру аккуратно.\n");
			break;
		}
		show_progress(cfg, &gs);
		printf("Ваш ответ (или 0 — сдаться): ");
		fflush(stdout);
		int guess = read_int_range(NULL, 0, cfg->max_number);
		if (guess == 0) break;
		gs.attempts++;
		if (gs.history_sz < gs.history_cap) gs.history[gs.history_sz++] = guess;
		if (guess == gs.target) {
			cprintf(C_GREEN, C_BOLD "Верно! Поздравляю!\n" C_RESET);
			won = true;
			break;
		}
		show_hint(&gs, guess);
	}
	analyze_and_print(cfg, &gs, won);
	ps->games++;
	if (won) ps->wins++;
	update_rating(ps, cfg, &gs, won);
	stats_save(ps);
	gs_free(&gs);
	puts("");
	wait_enter("Нажмите Enter, чтобы продолжить...\n");
}

int main(void) {
	enable_ansi_if_needed();
	seed_rng();
	setup_signals();
	clear_screen();
	header();
	player_stats_t stats;
	stats_load(&stats);
	game_config_t cfg = { .difficulty = 2 };
	config_by_difficulty(&cfg);
	for (;;) {
		int choice = menu();
		switch (choice) {
			case 1: play_game(&stats, &cfg); break;
			case 2: choose_difficulty(&cfg); break;
			case 3: show_stats(&stats); break;
			case 4: reset_stats(&stats); break;
			case 5: cprintf(C_CYAN, "Пока!\n"); return 0;
		}
	}
}
