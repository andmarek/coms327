#include <getopt.h>
#include <limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "cerr.h"
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

static void	sleep_next(WINDOW *const, int const, int const, char const *const);

static int	register_tiles(WINDOW *const, int const, int const);

static void	arrange_new(WINDOW *const, int const, int const);
static void	arrange_loaded(WINDOW *const, int const, int const);

static void	gen(void);

static void	print_nontunneling(WINDOW *const, int const, int const, int const, int const);
static void	print_tunneling(WINDOW *const, int const, int const, int const, int const);

int
main(int const argc, char *const argv[])
{
	WINDOW *win;
	int ch;
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

	if (refresh() == ERR) {
		cerrx(1, "refresh from initscr");
	}

	if ((win = newwin(height, width, 0, 0)) == NULL) {
		cerrx(1, "newwin");
	}

	/* always returns OK */
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
		if (load_dungeon(width, height) == -1) {
			cerrx(1, "loading dungeon");
		}

		arrange_loaded(win, width, height);

		(void)mvwaddch(win, p.y, p.x, PLAYER);
	} else {
		room_count = 8;
		stair_up_count = (uint16_t)rrand(1, (room_count / 4) + 1);
		stair_dn_count = (uint16_t)rrand(1, (room_count / 4) + 1);

		gen();
		arrange_new(win, width, height);

		if (register_tiles(win, width, height) == -1) {
			cerrx(1, "registering tiles");
		}

		place_player(win, &p, width, height);
	}

	if (!load) {
		(void)mvwprintw(win, height - 1, 26, "[seed: %u]", seed);
	}

	if (wrefresh(win) == ERR) {
		cerrx(1, "wrefresh on first draw cycle");
	}

	sleep_next(win, 'n', height, "press 'n' to proceed");

	if (dijkstra(width, height, p.y, p.x) == -1) {
		cerrx(1, "dijkstra");
	}

	print_nontunneling(win, width, height, p.y, p.x);

	sleep_next(win, 'n', height, "press 'n' to proceed");

	print_tunneling(win, width, height, p.y, p.x);

	sleep_next(win, 'q', height, "press 'q' to quit");

	if (delwin(win) == ERR) {
		cerrx(1, "delwin");
	}

	if (endwin() == ERR) {
		cerrx(1, "endwin");
	}

	if (!load) {
		printf("seed: %u\n", seed);
	}

	if (save && save_dungeon(width, height) == -1) {
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
sleep_next(WINDOW *const win, int const key, int const h, char const *const m)
{
	(void)box(win, 0, 0);
	(void)mvwprintw(win, h - 1, 2, "[%s]", m);

	if (wrefresh(win) == ERR) {
		cerrx(1, "sleep_next wrefresh");
	}

	int ch;

	while((ch = getch()) != key) {
		if (ch == ERR) {
			cerr(1, "sleep_next getch");
		}

		if (ch == KEY_RESIZE) {
			(void)mvwprintw(win, 0, 2,
				"[resizing the screen is undefined behavior]");
			(void)mvwprintw(win, h - 1, 2, "[%s]", m);

			if (wrefresh(win) == ERR) {
				cerrx(1, "sleep_next resize wrefresh");
			}
		}
	}
}

static void
usage(int const status, char const *const n)
{
	(void)printf("Usage: %s [OPTION]... \n\n", n);

	if (status != EXIT_SUCCESS) {
		(void)fprintf(stderr,
			"Try '%s --help' for more information.\n", n);
	} else {
		(void)puts("Generate a dungeon.\n");
		(void)puts("Options:\n\
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
	uint8_t i, j;

	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
			tiles[i][j].c = mvwinch(win, i, j);

			if (i == 0 || j == 0 || i == h - 1 || j == w - 1) {
				tiles[i][j].h = UINT8_MAX;
				continue;
			}

			tiles[i][j].y = i;
			tiles[i][j].x = j;

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

static void
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

		if (!rooms && room_count != 0) {
			cerr(1, "realloc rooms");
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
}

static void
arrange_loaded(WINDOW *const win, int const w, int const h)
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

	for (i = 1; i < h - 1; ++i) {
		for (j = 1; j < w - 1; ++j) {
			ch = mvwinch(win, i, j);

			tiles[i][j].y = i;
			tiles[i][j].x = j;

			if (tiles[i][j].h == 0 && ch == ROCK) {
				(void)mvwaddch(win, i, j, CORRIDOR);
				ch = mvwinch(win, i, j);
			}

			tiles[i][j].c = ch;
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

static void
print_nontunneling(WINDOW *const win, int const w, int const h, int const py, int const px)
{
	int i, j;

	for (i = 1; i < h - 1; ++i) {
		for (j = 1; j < w - 1; ++j) {
			if (i == py && j == px) {
				(void)mvwaddch(win, i, j, '@');
			} else if(tiles[i][j].h == 0) {
				if (tiles[i][j].d == INT32_MAX) {
					(void)mvwaddch(win, i, j, 'X');
				} else {
					(void)mvwprintw(win, i, j, "%d",
						tiles[i][j].d % 10);
				}
			} else {
				(void)mvwaddch(win, i, j, ' ');
			}
		}
	}

	if (wrefresh(win) == ERR) {
		cerrx(1, "print_nontunneling wrefresh");
	}
}

static void
print_tunneling(WINDOW *const win, int const w, int const h, int const py, int const px)
{
	int i, j;

	for (i = 1; i < h - 1; ++i) {
		for (j = 1; j < w - 1; ++j) {
			if (i == py && j == px) {
				(void)mvwaddch(win, i, j, '@');
			} else {
				(void)mvwprintw(win, i, j, "%d",
					tiles[i][j].dt % 10);
			}
		}
	}

	if (wrefresh(win) == ERR) {
		cerrx(1, "print_tunneling wrefresh");
	}
}
