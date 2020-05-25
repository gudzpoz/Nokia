GCC = gcc
SDL_LIB = -L/usr/lib -L/usr/local/lib -L./release -lSDL2 -lSDL2_ttf -lSDL2_mixer -Wl,-rpath=/usr/local/lib
SDL_INCLUDE = -I/usr/local/include
CFLAGS = -O3 -Wall -c -std=c99 $(SDL_INCLUDE)
LDFLAGS = $(SDL_LIB) -lm -Wl,-rpath,.
EXE = Nokia

all: $(EXE)

$(EXE): console.o util.o game.o main.o

	$(GCC) $^ $(LDFLAGS) -o $@
	cp $@ release/$@

main.o: main.c

	$(GCC) $(CFLAGS) $< -o $@

console.o: console.c console.h

	$(GCC) $(CFLAGS) $< -o $@

game.o: game.c game.h

	$(GCC) $(CFLAGS) $< -o $@

util.o: util.c util.h

	$(GCC) $(CFLAGS) $< -o $@

clean:

	rm *.o && rm $(EXE)
