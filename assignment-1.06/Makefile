CC = g++

CFLAGS := -g
CFLAGS += -std=c++17 -pedantic
CFLAGS += -Wextra -Wall -Wshadow -Wconversion

CFLAGS += -ftrapv -fverbose-asm
CFLAGS += -Wundef -Wcast-align=strict -Wstrict-overflow=5
CFLAGS += -Wwrite-strings -Wcast-qual -Wformat=2
CFLAGS += -Wswitch-enum -Wunreachable-code
CFLAGS += -pthread

FAST_CFLAGS := -O3 -std=c++17 -pedantic -pthread

BENCH_CFLAGS := -pg -fprofile-arcs -ftest-coverage -pthread

CFLAGS_END := -lncurses

DIRTY = *.gcda *.gcno *.gcov *.out error vgcore.*

src = dijk.cpp cerr.cpp floor.cpp gen.cpp io.cpp npc.cpp opal.cpp
hdr = dijk.h cerr.h floor.h gen.h globs.h io.h npc.h rand.h

opal: $(src) $(hdr)
	$(CC) $(FAST_CFLAGS) -o opal.out $(src) $(CFLAGS_END)

debug:
	$(CC) $(CFLAGS) -o opal.out $(src) $(CFLAGS_END)

bench:
	$(CC) $(BENCH_CFLAGS) -o opal.out $(src) $(CFLAGS_END)

clean:
	rm -f $(DIRTY)

val:
	valgrind --track-origins=yes ./opal.out
