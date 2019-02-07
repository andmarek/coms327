CC = gcc

CFLAGS := -g
CFLAGS += -std=c99 -pedantic
CFLAGS += -Wextra -Wall -Wshadow -Wconversion

CFLAGS += -ftrapv -fverbose-asm
CFLAGS += -Wundef -Wcast-align=strict -Wstrict-prototypes -Wstrict-overflow=5
CFLAGS += -Wwrite-strings -Waggregate-return -Wcast-qual -Wformat=2
CFLAGS += -Wswitch-default -Wswitch-enum -Wunreachable-code

CFLAGS_END := -lncurses

DIRTY = *.out vgcore.* error

src = floor.c gen.c hash.c io.c opal.c player.c rand.c
hdr = floor.h gen.h hash.h io.h opal.h player.h rand.h

opal: $(src) $(hdr)
	$(CC) $(CFLAGS) -o opal.out $(src) $(CFLAGS_END)

clean:
	rm -f $(DIRTY)

val:
	valgrind --track-origins=yes ./opal.out
