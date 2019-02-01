#include <ncurses.h>
#include <stdlib.h>

#include "rand.h"
#include "room.h"

#define ROOMCOUNT	8
#define ROOMRETRIES	150

#define STAIR_UPCOUNT	rrand(1, (ROOMCOUNT / 4) + 1)
#define STAIR_DOWNCOUNT	rrand(1, (ROOMCOUNT / 4) + 1)

int
main(int argc, char *argv[])
{
	WINDOW *win;
	int h, w, ch;
	unsigned int seed;
	struct room rooms[ROOMCOUNT];
	size_t i, retries;

	seed = init_rand(argc >= 2 ? argv[1] : NULL);

	win = initscr();
	box(win, 0, 0);
	getmaxyx(win, h, w);
	noecho();
	raw();

	mvprintw(h-2, 2, "Press 'q' to quit.");

	for (i = 0; i < ROOMCOUNT; ++i) {
		gen_room(&rooms[i], w, h);
	}

	retries = 0;

	for (i = 0; i < ROOMCOUNT && retries < ROOMRETRIES; ++i) {
		if (!draw_room(win, rooms[i])) {
			retries++;
			gen_room(&rooms[i--], w, h);
		}
	}

	for (i = 0; i < ROOMCOUNT - 1; ++i) {
		draw_corridor(win, rooms[i], rooms[i+1]);
	}

	for (i = 0; i < (size_t)STAIR_UPCOUNT; ++i) {
		draw_stair(win, w, h, true);
	}

	for (i = 0; i < (size_t)STAIR_DOWNCOUNT; ++i) {
		draw_stair(win, w, h, false);
	}

	wrefresh(win);

	while ((ch = getch()) != 'q');

	clear();
	endwin();

	printf("seed: %u\n", seed);

	return EXIT_SUCCESS;
}
