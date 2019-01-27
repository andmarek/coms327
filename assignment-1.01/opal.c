#include <ncurses.h>
#include <stdlib.h>

#include "rand.h"
#include "room.h"

#define HEIGHT	21
#define WIDTH	80

#define ROOMCOUNT	8

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
	size_t i;

	seed = init_rand(argc >= 2 ? argv[1] : NULL);

	initscr();
	raw();
	refresh();

	win = gen_screen();

	for (i = 0; i < ROOMCOUNT; ++i) {
		gen_room(&rooms[i], WIDTH, HEIGHT);
	}

	for (i = 0; i < ROOMCOUNT; ++i) {
		if (!draw_room(win, rooms[i])) {
			gen_room(&rooms[i--], WIDTH, HEIGHT);
		}
	}

	wrefresh(win);

	getch();

	delwin(win);
	clear();
	endwin();

	printf("seed: %u\n", seed);

	return EXIT_SUCCESS;
}
