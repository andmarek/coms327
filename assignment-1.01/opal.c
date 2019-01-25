#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>

#include "opal.h"

#define HEIGHT	21
#define WIDTH	80

#define RRANDH	rrand(1, HEIGHT - 2)	/* rrand in box height */
#define RRANDW	rrand(1, WIDTH - 2)	/* rrand in box width */

#define FILL	45	/* fill percentage */

#define ROOM		'.'
#define CORRIDOR	'#'
#define ROCK		' '
#define STAIR_UP	'<'
#define STAIR_DOWN	'>'

WINDOW *
generate_screen(void)
{
	WINDOW *win;

	win = newwin(HEIGHT, WIDTH, 0, 0);

	box(win, 0, 0);

	wrefresh(win);

	return win;
}

void
fill_floor(WINDOW *win)
{
	if (rrand(0, 100) < FILL) {
		mvwaddch(win, RRANDH, RRANDW, ROOM);
		wrefresh(win);
	}
}

size_t
wall_count(WINDOW *win, int x, int y)
{
	int i, j;
	size_t c = 0;

	for (i = x - 1; i <= x + 1; ++i) {
		for (j = y - 1; j <= y + 1; ++j) {
			if (mvwinch(win, j, i) == ROOM) c++;
		}
	}

	return c;
}

void
smooth_floor(WINDOW *win)
{
	int i, j;
	size_t wc;

	for (i = 1; i < WIDTH - 1; ++i) {
		for (j = 1; j < HEIGHT - 1; ++j) {
			wc = wall_count(win, i, j);

			if (wc < 4) {
				mvwaddch(win, j, i, ROCK);
			} else if (wc > 4) {
				mvwaddch(win, j, i, ROOM);
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	WINDOW *win;
	unsigned int seed;
	int i;

	seed = init_rand(argc >= 2 ? argv[1] : NULL);

	initscr();
	raw();

	refresh();

	win = generate_screen();

	for (i = 0; i < 1800; ++i) {
		if (argc <= 2) usleep(100);
		fill_floor(win);
	}

	for (i = 0; i < 5; ++i) {
		smooth_floor(win);
	}

	wrefresh(win);

	getch();

	delwin(win);

	clear();

	endwin();

	printf("seed: %u\n", seed);

	return EXIT_SUCCESS;
}
