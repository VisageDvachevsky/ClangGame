CC = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -pedantic
LDFLAGS = -lm
SRC = src/main.c src/game.c src/gameplay.c src/stats.c src/system.c src/terminal.c src/ui.c
OBJ = $(SRC:.c=.o)
INC = -Iinclude

clang_game: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(OBJ) clang_game
