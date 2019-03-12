CC = g++

CFLAGS := -g
CFLAGS += -std=c++17 -pedantic
CFLAGS += -Wextra -Wall -Wshadow -Wconversion

CFLAGS += -ftrapv -fverbose-asm
CFLAGS += -Wundef -Wcast-align=strict -Wstrict-overflow=5
CFLAGS += -Wwrite-strings -Wcast-qual -Wformat=2
CFLAGS += -Wswitch-enum -Wunreachable-code

CFLAGS_END := -lncurses

DIRTY = *.out vgcore.* error

src = dijk.cpp cerr.cpp floor.cpp gen.cpp heap.cpp io.cpp npc.cpp opal.cpp
hdr = dijk.h cerr.h floor.h gen.h globs.h heap.h io.h npc.h rand.h

opal: $(src) $(hdr)
	$(CC) $(CFLAGS) -o opal.out $(src) $(CFLAGS_END)

clean:
	rm -f $(DIRTY)

val:
	valgrind --track-origins=yes ./opal.out
