#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "cerr.h"
#include "dijk.h"
#include "floor.h"
#include "globs.h"
#include "io.h"
#include "npc.h"
#include "rand.h"

#define PROGRAM_NAME	"opal"
#define ROOM_RETRIES	150
#define WAIT_DEFAULT	250000

static struct option const long_opts[] = {
	{"help", no_argument, NULL, 'h'},
	{"load", no_argument, NULL, 'l'},
	{"nummon", required_argument, NULL, 'n'},
	{"save", no_argument, NULL, 's'},
	{"seed", required_argument, NULL, 'z'},
	{"wait", required_argument, NULL, 'w'},
	{NULL, 0, NULL, 0}
};

uint16_t room_count;
struct room *rooms;

uint16_t stair_up_count;
uint16_t stair_dn_count;

struct stair *stairs_up;
struct stair *stairs_dn;

struct tile tiles[HEIGHT][WIDTH];

static void	usage(int const, char const *const);

static void	handle_input(WINDOW *const, int const, char const *const);

static void	register_tiles(WINDOW *const);

static void	arrange_new(WINDOW *const);
static void	arrange_loaded(WINDOW *const);

static void	gen(void);

int
main(int const argc, char *const argv[])
{
	WINDOW *win;
	char *end;
	int ch;
	int load = 0;
	int save = 0;
	unsigned int nummon = (unsigned int)rrand(3, 5);
	unsigned int seed = UINT_MAX;
	unsigned int wait = WAIT_DEFAULT;
	char const *const name = (argc == 0) ? PROGRAM_NAME : argv[0];

	while ((ch = getopt_long(argc, argv, "hln:sz:w:", long_opts, NULL)) != -1) {
		switch(ch) {
		case 'h':
			usage(EXIT_SUCCESS, name);
			break;
		case 'l':
			load = 1;
			break;
		case 'n':
			nummon = (unsigned int)strtoul(optarg, &end, 10);

			if (optarg == end || errno == EINVAL || errno == ERANGE) {
				cerr(1, "nummon invalid");
			}
			break;
		case 's':
			save = 1;
			break;
		case 'z':
			seed = init_rand(optarg);
			break;
		case 'w':
			wait = (unsigned int)strtoul(optarg, &end, 10);

			if (optarg == end || errno == EINVAL || errno == ERANGE) {
				cerr(1, "wait invalid");
			}
			break;
		default:
			usage(EXIT_FAILURE, name);
		}
	}

	if (seed == UINT_MAX) {
		seed = init_rand(NULL);
	}

	(void)initscr();

	if (refresh() == ERR) {
		cerrx(1, "refresh from initscr");
	}

	if ((win = newwin(HEIGHT, WIDTH, 0, 0)) == NULL) {
		cerrx(1, "newwin");
	}

	(void)box(win, 0, 0);

	if (curs_set(0) == ERR) {
		cerrx(1, "curs_set");
	}

	if (noecho() == ERR) {
		cerrx(1, "noecho");
	}

	if (raw() == ERR) {
		cerrx(1, "raw");
	}

	if (load) {
		if (load_dungeon() == -1) {
			cerrx(1, "loading dungeon");
		}

		arrange_loaded(win);
	} else {
		room_count = 8;
		stair_up_count = (uint16_t)rrand(1, (room_count / 4) + 1);
		stair_dn_count = (uint16_t)rrand(1, (room_count / 4) + 1);

		gen();
		arrange_new(win);

		register_tiles(win);

		place_player();
	}

	if (!load) {
		(void)mvwprintw(win, HEIGHT - 1, 26, "[seed: %u]", seed);
	}

	dijkstra();

	handle_input(win, 'n', "['n' to continue]");

	turn_engine(win, wait, nummon);

	handle_input(win, 'q', "['q' to quit]");

	if (delwin(win) == ERR) {
		cerrx(1, "delwin");
	}

	if (endwin() == ERR) {
		cerrx(1, "endwin");
	}

	if (!load) {
		printf("seed: %u\n", seed);
	}

	if (save && save_dungeon() == -1) {
		cerrx(1, "saving dungeon");
	}

	/* zero memory before freeing, won't happen on erroneous exit */
	(void)memset(rooms, 0, sizeof(struct room) * room_count);
	(void)memset(stairs_up, 0, sizeof(struct stair) * stair_up_count);
	(void)memset(stairs_dn, 0, sizeof(struct stair) * stair_dn_count);

	free(rooms);
	free(stairs_up);
	free(stairs_dn);

	return EXIT_SUCCESS;
}

static void
handle_input(WINDOW *const win, int const disrupt,
	char const *const str)
{
	int ch;

	(void)box(win, 0, 0);
	(void)mvwprintw(win, HEIGHT - 1, 2, str);

	if (wrefresh(win) == ERR) {
		cerrx(1, "sleep_next wrefresh");
	}

	while((ch = getch()) != disrupt) {
		switch (ch) {
		case ERR:
			cerr(1, "handle_input getch");
			break;
		case KEY_RESIZE:
			(void)mvwprintw(win, 0, 2,
				"[resizing the screen is undefined behavior]");
			(void)mvwprintw(win, HEIGHT - 1, 2, str);

			if (wrefresh(win) == ERR) {
				cerrx(1, "handle_input getch");
			}
		}

	}

	(void)box(win, 0, 0);
}

static void
usage(int const status, char const *const name)
{
	(void)printf("Usage: %s [OPTION]... \n\n", name);

	if (status != EXIT_SUCCESS) {
		(void)fprintf(stderr,
			"Try '%s --help' for more information.\n", name);
	} else {
		(void)puts("Generate a dungeon.\n");
		(void)puts("Options:\n\
  -h, --help            display this help text and exit\n\
  -l, --load            load dungeon file\n\
  -n, --nummon=[NUM]    number of monsters\n\
  -s, --save            save dungeon file\n\
  -z, --seed=[SEED]     set rand seed, takes integer or string");
	}

	exit(status);
}

static void
register_tiles(WINDOW *const win)
{
	uint8_t i, j;

	for (i = 0; i < HEIGHT; ++i) {
		for (j = 0; j < WIDTH; ++j) {
			tiles[i][j].n = NULL;
			tiles[i][j].c = mvwinch(win, i, j);

			if (i == 0 || j == 0 || i == HEIGHT - 1
				|| j == WIDTH - 1) {
				tiles[i][j].h = UINT8_MAX;
				continue;
			}

			tiles[i][j].y = i;
			tiles[i][j].x = j;

			switch(tiles[i][j].c) {
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
}

static void
arrange_new(WINDOW *const win)
{
	size_t i, retries = 0;

	for (i = 0; i < room_count; ++i) {
		gen_room(&rooms[i]);
	}

	for (i = 0; i < room_count && retries < ROOM_RETRIES; ++i) {
		if (valid_room(win, &rooms[i]) == -1) {
			retries++;
			gen_room(&rooms[i--]);
		} else {
			draw_room(win, &rooms[i]);
		}
	}

	if (i < room_count) {
		room_count = (uint16_t)i;
		rooms = realloc(rooms, sizeof(struct room) * room_count);

		if (!rooms && room_count != 0) {
			cerr(1, "realloc rooms");
		}
	}

	for (i = 0; i < room_count - 1U; ++i) {
		draw_corridor(win, rooms[i], rooms[i+1]);
	}

	for (i = 0; i < stair_up_count; ++i) {
		gen_draw_stair(win, &stairs_up[i], true);
	}

	for (i = 0; i < stair_dn_count; ++i) {
		gen_draw_stair(win, &stairs_dn[i], false);
	}
}

static void
arrange_loaded(WINDOW *const win)
{
	chtype ch;
	uint8_t i, j;
	uint16_t k;

	for (k = 0; k < room_count; ++k) {
		draw_room(win, &rooms[k]);
	}

	for (k = 0; k < stair_up_count; ++k) {
		(void)mvwaddch(win, stairs_up[k].y, stairs_up[k].x, STAIR_UP);
	}

	for (k = 0; k < stair_dn_count; ++k) {
		(void)mvwaddch(win, stairs_dn[k].y, stairs_dn[k].x, STAIR_DN);
	}

	for (i = 1; i < HEIGHT - 1; ++i) {
		for (j = 1; j < WIDTH - 1; ++j) {
			ch = mvwinch(win, i, j);

			tiles[i][j].y = i;
			tiles[i][j].x = j;

			if (tiles[i][j].h == 0 && ch == ROCK) {
				(void)mvwaddch(win, i, j, CORRIDOR);
				ch = mvwinch(win, i, j);
			}

			tiles[i][j].c = ch;
			tiles[i][j].n = NULL;
		}
	}
}

static void
gen(void)
{
	if (!(rooms = malloc(sizeof(struct room) * room_count))
		&& room_count != 0) {
		cerr(1, "gen malloc rooms");
	}

	if (!(stairs_up = malloc(sizeof(struct stair) * stair_up_count))
		&& stair_up_count != 0) {
		cerr(1, "gen malloc stairs_dn");
	}

	if (!(stairs_dn = malloc(sizeof(struct stair) * stair_dn_count))
		&& stair_dn_count != 0) {
		cerr(1, "gen malloc stairs_dn");
	}
}
