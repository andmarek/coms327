#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "cerr.h"
#include "gen.h"
#include "globs.h"
#include "io.h"
#include "npc.h"
#include "rand.h"

#define PROGRAM_NAME	"opal"

static struct option const long_opts[] = {
	{"help", no_argument, NULL, 'h'},
	{"load", no_argument, NULL, 'l'},
	{"nummon", required_argument, NULL, 'n'},
	{"save", no_argument, NULL, 's'},
	{"seed", required_argument, NULL, 'z'},
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

/*
static void	handle_input(WINDOW *const, int const, char const *const);
*/

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

	if (keypad(win, true) == ERR) {
		cerrx(1, "keypad");
	}

	clear_tiles();

	if (load) {
		if (load_dungeon() == -1) {
			cerrx(1, "loading dungeon");
		}

		arrange_loaded(win);
	} else {
		arrange_new(win);
	}

	/* handle_input(win, 'n', "['n' to continue]"); */

	turn_engine(win, nummon);

	/* handle_input(win, 'q', "['q' to quit]"); */

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

/*
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

	while((ch = wgetch(win)) != disrupt) {
		switch (ch) {
		case ERR:
			cerr(1, "handle_input wgetch");
			break;
		case KEY_RESIZE:
			(void)mvwprintw(win, 0, 2,
				"[resizing the screen is undefined behavior]");
			(void)mvwprintw(win, HEIGHT - 1, 2, str);

			if (wrefresh(win) == ERR) {
				cerrx(1, "handle_input resize wrefresh");
			}
		}

	}

	(void)box(win, 0, 0);
}
*/

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
