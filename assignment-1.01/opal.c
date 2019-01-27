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

#define ROOMCOUNT	4

#define MINROOMH	4
#define MINROOMW	3
#define MAXROOMH	8
#define MAXROOMW	30

struct room {
	int x;
	int y;
	int size_x;
	int size_y;
};

WINDOW *
gen_screen(void)
{
	WINDOW *win;

	win = newwin(HEIGHT, WIDTH, 0, 0);

	box(win, 0, 0);

	wrefresh(win);

	return win;
}

void
gen_room(struct room *r)
{
	r->x = rrand(1, WIDTH - MAXROOMW - 2);
	r->y = rrand(1, HEIGHT - MAXROOMH - 2);
	r->size_x = rrand(MINROOMW, MAXROOMW);
	r->size_y = rrand(MINROOMH, MAXROOMH);
}

bool
draw_room(WINDOW *win, struct room r, chtype tile)
{
	int i, j;

	for (i = r.x - 1; i <= r.x + r.size_x + 1; ++i) {
		for (j = r.y - 1; j <= r.y + r.size_y + 1; ++j) {
			if (mvwinch(win, j, i) != ROCK) {
				return false;
			}
		}
	}

	for (i = r.x; i <= r.x + r.size_x; ++i) {
		for (j = r.y; j <= r.y + r.size_y; ++j) {
			mvwaddch(win, j, i, tile);
			wrefresh(win);
		}
	}

	return true;
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
		gen_room(&rooms[i]);
	}

	for (i = 0; i < ROOMCOUNT; ++i) {
		if (!draw_room(win, rooms[i], ROOM)) {
			gen_room(&rooms[i--]);
		}
	}

	getch();

	delwin(win);

	clear();

	endwin();

	printf("seed: %u\n", seed);

	return EXIT_SUCCESS;
}
