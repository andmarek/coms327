CC = gcc

CFLAGS := -g
CFLAGS += -std=c99 -pedantic
CFLAGS += -Wextra -Wall -Wshadow -Wconversion

CFLAGS += -ftrapv -fverbose-asm
CFLAGS += -Wundef -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5
CFLAGS += -Wwrite-strings -Waggregate-return -Wcast-qual -Wformat=2
CFLAGS += -Wswitch-default -Wswitch-enum -Wunreachable-code

CFLAGS_END := -lncurses

DIRTY = *.out vgcore.*

opal: hash.c rand.c opal.c opal.h
	$(CC) $(CFLAGS) -o opal.out hash.c rand.c opal.c $(CFLAGS_END)

clean:
	rm -f $(DIRTY)
