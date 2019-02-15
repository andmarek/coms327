#include <getopt.h>
#include <limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "dijk.h"
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

uint16_t room_count;
struct room *rooms;

uint16_t stair_up_count;
uint16_t stair_dn_count;

struct stair *stairs_up;
struct stair *stairs_dn;

struct tile tiles[HEIGHT][WIDTH];

static void	usage(int const, char const *const);

static int	register_tiles(WINDOW *const, int const, int const);

static int	arrange_new(WINDOW *const, int const, int const);
static void	arrange_loaded(WINDOW *const, int const, int const);

static int	gen(void);

int
main(int const argc, char *const argv[])
{
	WINDOW *win;
	int ch, ok = EXIT_SUCCESS;
	int load = 0, save = 0;
	unsigned int seed = UINT_MAX;
	char const *const name = (argc == 0) ? PROGRAM_NAME : argv[0];

	int const width = WIDTH, height = HEIGHT;

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
		if (load_dungeon(width, height) == -1) {
			fputs("error loading dungeon\n", stderr);
			ok = EXIT_FAILURE;
			goto exit;
		}

		arrange_loaded(win, width, height);

		mvwaddch(win, p.y, p.x, PLAYER);
	} else {
		room_count = 8;
		stair_up_count = (uint16_t)rrand(1, (room_count / 4) + 1);
		stair_dn_count = (uint16_t)rrand(1, (room_count / 4) + 1);

		if (gen() == -1 || arrange_new(win, width, height) == -1) {
			fputs("error generating dungeon\n", stderr);
			ok = EXIT_FAILURE;
			goto exit;
		}

		if (register_tiles(win, width, height) == -1) {
			fputs("error registering tiles\n", stderr);
			ok = EXIT_FAILURE;
			goto exit;
		}

		place_player(win, &p, width, height);
	}

	mvwprintw(win, height - 1, 2, "[press 'q' to quit]");

	if (!load) {
		mvwprintw(win, height - 1, 26, "[seed: %u]", seed);
	}

	wrefresh(win);

	while ((ch = getch()) != 'q') {
		if (ch == KEY_RESIZE) {
			mvprintw(0, 2,
				"[resizing the screen is undefined behavior]");
		}
	}

	delwin(win);
	endwin();
	win = NULL;

	if (!load) {
		printf("seed: %u\n", seed);
	}

	if (save && save_dungeon(width, height) == -1) {
		fprintf(stderr, "error saving dungeon\n");
		ok = EXIT_FAILURE;
		goto exit;
	}

	if (dijkstra(width, height, p.y, p.x) == -1) {
		fprintf(stderr, "error in dijstra\n");
		ok = EXIT_FAILURE;
		goto exit;
	}

	print_nontunneling(width, height, p.y, p.x);
	print_tunneling(width, height, p.y, p.x);

	exit:

	if (win != NULL) {
		delwin(win);
		endwin();
	}

	/* zero memory before freeing */
	memset(rooms, 0, sizeof(struct room) * room_count);
	memset(stairs_up, 0, sizeof(struct stair) * stair_up_count);
	memset(stairs_dn, 0, sizeof(struct stair) * stair_dn_count);

	free(rooms);
	free(stairs_up);
	free(stairs_dn);

	return ok;
}

static void
usage(int const status, char const *const n)
{
	printf("Usage: %s [OPTION]... \n\n", n);

	if (status != EXIT_SUCCESS) {
		fprintf(stderr, "Try '%s --help' for more information.\n", n);
	} else {
		puts("Generate a dungeon.\n");
		puts("Options:\n\
  -h, --help           display this help text and exit\n\
  -l, --load           load dungeon file\n\
  -s, --save           save dungeon file\n\
  -z, --seed=[SEED]    set rand seed, takes integer or string");
	}

	exit(status);
}

static int
register_tiles(WINDOW *const win, int const w, int const h)
{
	int i, j;

	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
			tiles[i][j].c = mvwinch(win, i, j);

			if (i == 0 || j == 0 || i == h - 1 || j == w - 1) {
				tiles[i][j].h = UINT8_MAX;
				continue;
			}

			tiles[i][j].y = (uint8_t)i;
			tiles[i][j].x = (uint8_t)j;

			switch(tiles[i][j].c) {
			case PLAYER:
				return -1;
			case ROOM:
			case CORRIDOR:
			case STAIR_UP:
			case STAIR_DN:
				tiles[i][j].h = 0;
				break;
			default:
				tiles[i][j].h = (uint8_t)rrand(1, UINT8_MAX-1U);
			}
		}
	}

	return 0;
}

static int
arrange_new(WINDOW *const win, int const w, int const h)
{
	size_t i, retries = 0;

	for (i = 0; i < room_count; ++i) {
		gen_room(&rooms[i], w, h);
	}

	for (i = 0; i < room_count && retries < ROOM_RETRIES; ++i) {
		if (valid_room(win, &rooms[i]) == -1) {
			retries++;
			gen_room(&rooms[i--], w, h);
		} else {
			draw_room(win, &rooms[i]);
		}
	}

	if (i < room_count) {
		room_count = (uint16_t)i;
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

static void
arrange_loaded(WINDOW *const win, int const w, int const h)
{
	chtype ch;
	int i, j;

	for (i = 0; i < room_count; ++i) {
		draw_room(win, &rooms[i]);
	}

	for (i = 0; i < stair_up_count; ++i) {
		mvwaddch(win, stairs_up[i].y, stairs_up[i].x, STAIR_UP);
	}

	for (i = 0; i < stair_dn_count; ++i) {
		mvwaddch(win, stairs_dn[i].y, stairs_dn[i].x, STAIR_DN);
	}

	for (i = 1; i < h - 1; ++i) {
		for (j = 1; j < w - 1; ++j) {
			ch = mvwinch(win, i, j);

			tiles[i][j].y = (uint8_t)i;
			tiles[i][j].x = (uint8_t)j;

			if (tiles[i][j].h == 0 && ch == ROCK) {
				mvwaddch(win, i, j, CORRIDOR);
				ch = mvwinch(win, i, j);
			}

			tiles[i][j].c = ch;
		}
	}
}

static int
gen(void)
{
	if (!(rooms = malloc(sizeof(struct room) * room_count))) {
		return -1;
	}

	if (!(stairs_up = malloc(sizeof(struct stair) * stair_up_count))) {
		return -1;
	}

	if (!(stairs_dn = malloc(sizeof(struct stair) * stair_dn_count))) {
		return -1;
	}

	return 0;
}
