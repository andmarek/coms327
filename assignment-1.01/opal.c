#include<ncurses.h>
#include<stdio.h>

#include "opal.h"

#define HEIGHT	21
#define WIDTH	80

#define RRANDH	rrand(1, HEIGHT - 1)	/* rrand in box height */
#define RRANDW	rrand(1, WIDTH - 1)		/* rrand in box width */

#define FILL	45	/* fill percentage */

#define ROOM		'.'
#define CORRIDOR	'#'
#define ROCK		' '
#define STAIR_UP	'<'
#define STAIR_DOWN	'>'

WINDOW *generate_screen(void)
{
	WINDOW *win;

	win = newwin(HEIGHT, WIDTH, 0, 0);

	box(win, 0, 0);

	wrefresh(win);

	return win;
}

void add_room(WINDOW *win)
{
	mvwaddch(win, RRANDH, RRANDW, ROOM);
	wrefresh(win);
}

int main(int argc, char *argv[])
{
	WINDOW *win;

	init_rand(argc >= 2 ? argv[1] : NULL);

	initscr();
	raw();

	refresh();

	win = generate_screen();

	add_room(win);

	getch();

	delwin(win);

	clear();

	endwin();

	return EXIT_SUCCESS;
}
