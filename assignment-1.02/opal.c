#include <ncurses.h>
#include <stdlib.h>

#include "rand.h"
#include "room.h"

#define ROOMCOUNT	8
#define ROOMRETRIES	150

#define STAIR_UPCOUNT	rrand(1, (ROOMCOUNT / 4) + 1)
#define STAIR_DOWNCOUNT	rrand(1, (ROOMCOUNT / 4) + 1)

static void
arrange_floor(WINDOW *const win, int const w, int const h,
	struct room *const rooms)
{
	size_t i, retries;

	for (i = 0; i < ROOMCOUNT; ++i) {
		gen_room(&rooms[i], w, h);
	}

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
}

int
main(int argc, char *argv[])
{
	WINDOW *win;
	struct room rooms[ROOMCOUNT];
	int h, w, ch;
	unsigned int const seed = init_rand(argc >= 2 ? argv[1] : NULL);

	win = initscr();
	box(win, 0, 0);
	getmaxyx(win, h, w);
	noecho();
	raw();

	mvprintw(h-2, 2, "Press 'q' to quit.");

	arrange_floor(win, w, h, rooms);

	while ((ch = getch()) != 'q') {
		if (ch == KEY_RESIZE) {
			mvprintw(1, 1, "Resizing screen is undefined behavior");
		}
	}

	endwin();

	printf("seed: %u\n", seed);

	return EXIT_SUCCESS;
}
