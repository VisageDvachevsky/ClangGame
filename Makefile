CC = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -pedantic
LDFLAGS = -lm
SRC = src/main.c src/game.c src/ui.c src/rng.c src/stats.c src/utils.c src/progress.c
OBJ = $(SRC:.c=.o)
INC = -Iinclude

clang_game: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.c include/game.h
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(OBJ) clang_game
