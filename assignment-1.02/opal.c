#include <getopt.h>
#include <limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "gen.h"
#include "io.h"
#include "opal.h"
#include "player.h"
#include "rand.h"

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

uint8_t room_count;
struct room *rooms;

uint8_t stair_up_count;
uint8_t stair_dn_count;

struct stair *stairs_up;
struct stair *stairs_dn;

int const width = 80;
int const height = 21;

struct tile *tiles;

static void
usage(int const status, char const *const n)
{
	printf("Usage: %s [OPTION]... \n\n", n);

	if (status != EXIT_SUCCESS) {
		fprintf(stderr, "Try '%s --help' for more information.\n", n);
	} else {
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
register_tiles(WINDOW *const win, int const w, int const h)
{
	int i, j;

	for (i = 0; i < w; ++i) {
		for (j = 0; j < h; ++j) {
			tiles[i*j].c = (unsigned char)mvwinch(win, j, i);

			if (i == 0 || j == 0 || i == w - 1 || j == h - 1) {
				tiles[i*j].h = UINT8_MAX;
				continue;
			}

			switch(tiles[i*j].c) {
				case ROOM: // fallthrough
				case CORRIDOR: // fallthrough
				case STAIR_UP: // fallthrough
				case STAIR_DN:
					tiles[i*j].h = 0;
					break;
				default:
					tiles[i*j].h = (uint8_t)rrand(1, UINT8_MAX - 1U);
			}
		}
	}
}

static int
arrange_floor(WINDOW *const win, int const w, int const h)
{
	size_t i, retries = 0;

	for (i = 0; i < room_count; ++i) {
		gen_room(&rooms[i], w, h);
	}

	for (i = 0; i < room_count && retries < ROOM_RETRIES; ++i) {
		if (!draw_room(win, &rooms[i])) {
			retries++;
			gen_room(&rooms[i--], w, h);
		}
	}

	if (i < room_count) {
		room_count = (uint8_t)i;
		rooms = realloc(rooms, sizeof(struct room) * room_count);

		if (!rooms) {
			return -1;
		}
	}

	for (i = 0; i < room_count - 1U; ++i) {
		draw_corridor(win, rooms[i], rooms[i+1]);
	}

	for (i = 0; i < stair_up_count; ++i) {
		gen_draw_stair(win, &stairs_up[i], w, h, true);
	}

	for (i = 0; i < stair_dn_count; ++i) {
		gen_draw_stair(win, &stairs_dn[i], w, h, false);
	}

	return 0;
}

int
main(int const argc, char *const argv[])
{
	WINDOW *win;
	int ch;
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

	initscr();
	refresh();
	win = newwin(height, width, 0, 0);
	box(win, 0, 0);
	curs_set(0);
	noecho();
	raw();

	if (load) {
		if (load_dungeon() == -1) {
			fputs("error loading dungeon\n", stderr);
			goto exit;
		}
	} else {
		room_count = 8;
		stair_up_count = (uint8_t)rrand(1, (room_count / 4) + 1);
		stair_dn_count = (uint8_t)rrand(1, (room_count / 4) + 1);

		if (gen() == -1 || arrange_floor(win, width, height) == -1) {
			fputs("error generating dungeon\n", stderr);
			goto exit;
		}

		register_tiles(win, width, height);

		place_player(win, &p, width, height);
	}

	mvwprintw(win, height - 1, 2, "[press 'q' to quit]");
	mvwprintw(win, height - 1, 26, "[seed: %u]", seed);

	wrefresh(win);

	while ((ch = getch()) != 'q') {
		if (ch == KEY_RESIZE) {
			mvprintw(0, 2,
				"[resizing the screen is undefined behavior]");
		}
	}

	if (!load) {
		printf("seed: %u\n", seed);
	}

	if (save && save_dungeon() == -1) {
		fprintf(stderr, "error saving dungeon\n");
	}

	exit:

	delwin(win);
	endwin();

	// zero memory before freeing
	memset(rooms, 0, sizeof(struct room) * room_count);
	memset(stairs_up, 0, sizeof(struct stair) * stair_up_count);
	memset(stairs_dn, 0, sizeof(struct stair) * stair_dn_count);
	memset(tiles, 0, sizeof(struct tile) * (size_t)width * (size_t)height);

	free(rooms);
	free(stairs_up);
	free(stairs_dn);
	free(tiles);

	return EXIT_SUCCESS;
}
