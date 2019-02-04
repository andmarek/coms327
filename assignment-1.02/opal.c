#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "opal.h"
#include "player.h"
#include "rand.h"

#undef FULLSCREEN

#define ROOM_RETRIES	150

struct player p;

uint8_t const ROOM_COUNT = 8;
struct room *rooms;

uint8_t stair_up_count;
uint8_t stair_dn_count;

struct stair *stairs_up;
struct stair *stairs_dn;

static void
arrange_floor(WINDOW *const win, int const w, int const h)
{
	size_t i, retries = 0;

	for (i = 0; i < ROOM_COUNT; ++i) {
		gen_room(&rooms[i], w, h);
	}

	for (i = 0; i < ROOM_COUNT && retries < ROOM_RETRIES; ++i) {
		if (!draw_room(win, &rooms[i])) {
			retries++;
			gen_room(&rooms[i--], w, h);
		}
	}

	for (i = 0; i < (size_t)ROOM_COUNT - 1; ++i) {
		draw_corridor(win, rooms[i], rooms[i+1]);
	}

	for (i = 0; i < stair_up_count; ++i) {
		draw_stair(win, &stairs_up[i], w, h, true);
	}

	for (i = 0; i < stair_dn_count; ++i) {
		draw_stair(win, &stairs_dn[i], w, h, false);
	}
}

struct tile {
	uint8_t	h;	/* hardness */
	chtype	c;	/* character */
};

static void
wipe(void)
{
	// zero memory before freeing
	memset(rooms, 0, sizeof(struct room) * ROOM_COUNT);
	memset(stairs_up, 0, sizeof(struct stair) * stair_up_count);
	memset(stairs_dn, 0, sizeof(struct stair) * stair_dn_count);

	free(rooms);
	free(stairs_up);
	free(stairs_dn);
}

int
main(int const argc, char const *const argv[])
{
	WINDOW *win;
	int h, w, ch;
	unsigned int const seed = init_rand(argc >= 2 ? argv[1] : NULL);

	if (!(rooms = malloc(sizeof(struct room) * ROOM_COUNT))) {
		fputs("error allocating memory for rooms", stderr);
		return EXIT_FAILURE;
	}

	stair_up_count = (uint8_t)rrand(1, (ROOM_COUNT / 4) + 1);
	stair_dn_count = (uint8_t)rrand(1, (ROOM_COUNT / 4) + 1);

	if (!(stairs_up = malloc(sizeof(struct stair) * stair_up_count))) {
		fputs("error allocating memory for stairs_up", stderr);
		return EXIT_FAILURE;
	}

	if (!(stairs_dn = malloc(sizeof(struct stair) * stair_dn_count))) {
		fputs("error allocating memory for stairs_dn", stderr);
		return EXIT_FAILURE;
	}

	atexit(wipe);

	win = initscr();

#ifdef FULLSCREEN
	getmaxyx(win, h, w);
#else
	refresh();
	h = 21;
	w = 80;
	win = newwin(h, w, 0, 0);
#endif

	box(win, 0, 0);
	curs_set(0);
	noecho();
	raw();

	mvwprintw(win, h - 1, 2, "[press 'q' to quit]");
	mvwprintw(win, h - 1, 26, "[seed: %u]", seed);

	arrange_floor(win, w, h);

	place_player(win, &p, w, h);

#ifndef FULLSCREEN
	wrefresh(win);
#endif

	while ((ch = getch()) != 'q') {
		if (ch == KEY_RESIZE) {
			mvprintw(0, 2,
				"[resizing the screen is undefined behavior]");
		}
	}

#ifndef FULLSCREEN
	delwin(win);
	clear();
#endif

	endwin();

	printf("seed: %u\n", seed);

	if (save_dungeon() == -1) {
		fprintf(stderr, "error saving dungeon\n");
	}

	return EXIT_SUCCESS;
}
