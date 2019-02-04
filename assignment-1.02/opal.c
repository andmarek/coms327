#include <getopt.h>
#include <limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "opal.h"
#include "player.h"
#include "rand.h"

#undef FULLSCREEN

#define PROGRAM_NAME	"opal"

#define ROOM_RETRIES	150

static struct option const long_opts[] = {
	{"help", no_argument, NULL, 'h'},
	{"load", no_argument, NULL, 'l'},
	{"save", no_argument, NULL, 's'},
	{"seed", required_argument, NULL, 'z'},
	{NULL, 0, NULL, 0}
};

struct player p;

uint8_t const ROOM_COUNT = 8;
struct room *rooms;

uint8_t stair_up_count;
uint8_t stair_dn_count;

struct stair *stairs_up;
struct stair *stairs_dn;

static void
usage(int const status, char const *const n)
{
	printf("Usage: %s [OPTION]... \n\n", n);

	if (status != EXIT_SUCCESS) {
		fprintf(stderr, "Try '%s --help' for more information.\n", n);
	} {
		puts("Generate a dungeon.\n");
		puts("Options:\n\
  -h, --help    display this help text and exit\n\
  -l, --load    load dungeon file\n\
  -s, --save    save dungeon file\n\
  -z, --seed    set rand seed");
	}

	exit(status);
}

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
main(int const argc, char *const argv[])
{
	WINDOW *win;
	int h, w, ch;
	int load = 0, save = 0;
	unsigned int seed = UINT_MAX;
	char const *const name = (argc == 0) ? PROGRAM_NAME : argv[0];

	while ((ch = getopt_long(argc, argv, "hlsz:", long_opts, NULL)) != -1) {
		switch(ch) {
		case 'h':
			usage(EXIT_SUCCESS, name);
			break;
		case 'l':
			load = 1;
			break;
		case 's':
			save = 1;
			break;
		case 'z':
			seed = init_rand(optarg);
			break;
		default:
			usage(EXIT_FAILURE, name);
		}
	}

	if (seed == UINT_MAX) {
		seed = init_rand(NULL);
	}

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
