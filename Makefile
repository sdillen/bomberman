.PHONY: all clean

CC = clang

CFLAGS = -Wall
CFLAGS += -isystem /nix/store/bvxjdpr4zq9r4a951340wn8h35xh02vb-clang-19.1.7-lib/lib/clang/19/include
CFLAGS += ${NIX_LDFLAGS} ${NIX_CFLAGS_COMPILE}
CFLAGS += -lraylib

OBJ = src/game.o src/renderer.o src/input.o src/log.o src/util.o
EXEC = main

all: $(EXEC)

$(EXEC): src/main.c src/game.o src/renderer.o src/input.o src/log.o src/util.o
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) src/main.c

src/game.o: src/game.c src/game.h
	$(CC) $(CFLAGS) -c src/game.c -o src/game.o

src/renderer.o: src/renderer.c src/renderer.h
	$(CC) $(CFLAGS) -c src/renderer.c -o src/renderer.o

src/input.o: src/input.c src/input.h
	$(CC) $(CFLAGS) -c src/input.c -o src/input.o

src/log.o: src/log.c src/log.h
	$(CC) $(CFLAGS) -c src/log.c -o src/log.o

src/util.o: src/util.c src/util.h
	$(CC) $(CFLAGS) -c src/util.c -o src/util.o

compile_commands.json: devenv.yaml devenv.nix clean
	bear -- make

clean:
	rm -fv $(EXEC)
	rm -fv src/*.o
	rm -fv src/*.so
	@echo Cleaning done
