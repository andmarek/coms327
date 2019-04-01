CXX = g++

CXXFLAGS := -g
CXXFLAGS += -std=c++17 -pedantic
CXXFLAGS += -Wextra -Wall -Wshadow -Wconversion

CXXFLAGS += -ftrapv -fverbose-asm
CXXFLAGS += -Wundef -Wcast-align=strict -Wstrict-overflow=5
CXXFLAGS += -Wwrite-strings -Wcast-qual -Wformat=2
CXXFLAGS += -Wswitch-enum -Wunreachable-code
CXXFLAGS += -pthread

FAST_CXXFLAGS := -O3 -std=c++17 -pedantic -pthread

BENCH_CXXFLAGS := -pg -fprofile-arcs -ftest-coverage -pthread

CXXFLAGS_END := -lncurses

DIRTY := *.gcda *.gcno *.gcov *.out error vgcore.*
DIRTY += *.tab.c *.tab.h lex.yy.c y.dot y.output

src = dijk.cpp cerr.cpp floor.cpp gen.cpp npc.cpp rand.cpp opal.cpp
hdr = dijk.h cerr.h floor.h gen.h globs.h npc.h rand.h

opal: $(src) $(hdr)
	$(CXX) $(FAST_CXXFLAGS) -o opal.out $(src) $(CXXFLAGS_END)

debug:
	$(CXX) $(CXXFLAGS) -o opal.out $(src) $(CXXFLAGS_END)

bench:
	$(CXX) $(BENCH_CXXFLAGS) -o opal.out $(src) $(CXXFLAGS_END)

clean:
	rm -f $(DIRTY)

val:
	valgrind -v --track-origins=yes --log-file=error ./opal.out

yl-monster-debug: monster.l monster.y
	lex -v -d monster.l
	yacc -v -d monster.y
	$(CXX) -g -Wextra -Wall -o monster.out y.tab.c lex.yy.c cerr.cpp

yl-monster: monster.l monster.y
	lex --fast monster.l
	yacc -d -l monster.y
	$(CXX) -O3 -o monster.out y.tab.c lex.yy.c cerr.cpp
