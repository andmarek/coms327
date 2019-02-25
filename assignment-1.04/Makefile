CC = gcc

CFLAGS := -g
CFLAGS += -std=c99 -pedantic
CFLAGS += -Wextra -Wall -Wshadow -Wconversion

CFLAGS += -ftrapv -fverbose-asm
CFLAGS += -Wundef -Wcast-align=strict -Wstrict-prototypes -Wstrict-overflow=5
CFLAGS += -Wwrite-strings -Waggregate-return -Wcast-qual -Wformat=2
CFLAGS += -Wswitch-default -Wswitch-enum -Wunreachable-code

CFLAGS_END := -lncurses -lm

DIRTY = *.out vgcore.* error

src = dijk.c cerr.c floor.c heap.c io.c npc.c opal.c rand.c
hdr = dijk.c cerr.h floor.h heap.h io.h npc.h opal.h rand.h

opal: $(src) $(hdr)
	$(CC) $(CFLAGS) -o opal.out $(src) $(CFLAGS_END)

clean:
	rm -f $(DIRTY)

val:
	valgrind --track-origins=yes ./opal.out
