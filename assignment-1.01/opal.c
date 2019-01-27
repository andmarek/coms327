#include <ncurses.h>
#include <stdlib.h>

#include "rand.h"
#include "room.h"

#define HEIGHT	21
#define WIDTH	80

#define ROOMCOUNT	8
#define ROOMRETRIES	150

#define STAIR_UPCOUNT	rrand(1, (ROOMCOUNT / 4) + 1)
#define STAIR_DOWNCOUNT	rrand(1, (ROOMCOUNT / 4) + 1)

WINDOW *
gen_screen(void)
{
	WINDOW *win;

	win = newwin(HEIGHT, WIDTH, 0, 0);

	box(win, 0, 0);

	wrefresh(win);

	return win;
}

int
main(int argc, char *argv[])
{
	WINDOW *win;
	unsigned int seed;
	struct room rooms[ROOMCOUNT];
	size_t i, retries;

	seed = init_rand(argc >= 2 ? argv[1] : NULL);

	initscr();
	raw();
	refresh();

	win = gen_screen();

	for (i = 0; i < ROOMCOUNT; ++i) {
		gen_room(&rooms[i], WIDTH, HEIGHT);
	}

	retries = 0;

	for (i = 0; i < ROOMCOUNT && retries < ROOMRETRIES; ++i) {
		if (!draw_room(win, rooms[i])) {
			retries++;
			gen_room(&rooms[i--], WIDTH, HEIGHT);
		}
	}

	for (i = 0; i < ROOMCOUNT - 1; ++i) {
		draw_corridor(win, rooms[i], rooms[i+1]);
	}

	for (i = 0; i < (size_t)STAIR_UPCOUNT; ++i) {
		draw_stair(win, WIDTH, HEIGHT, true);
	}

	for (i = 0; i < (size_t)STAIR_DOWNCOUNT; ++i) {
		draw_stair(win, WIDTH, HEIGHT, false);
	}

	wrefresh(win);

	getch();

	delwin(win);
	clear();
	endwin();

	printf("seed: %u\n", seed);

	return EXIT_SUCCESS;
}
