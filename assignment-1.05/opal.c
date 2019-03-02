#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

static void	print_deathscreen(WINDOW *const);
static void	print_winscreen(WINDOW *const);

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

	retry:
	switch(turn_engine(win, nummon)) {
		case TURN_DEATH:
			sleep(1);
			print_deathscreen(win);
			break;
		case TURN_NEXT:
			arrange_renew(win);
			goto retry;
			break;
		case TURN_NONE:
			cerrx(1, "turn_engine return value invalid");
			break;
		case TURN_QUIT:
			break;
		case TURN_WIN:
			sleep(1);
			print_winscreen(win);
			break;
	}

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
usage(int const status, char const *const name)
{
	(void)printf("Usage: %s [OPTION]... \n\n", name);

	if (status != EXIT_SUCCESS) {
		(void)fprintf(stderr,
			"Try '%s --help' for more information.\n", name);
	} else {
		(void)puts("OPAL's Playable Almost Indefectibly.\n");
		(void)puts("Traverse a generated dungeon.\n");
		(void)puts("Options:\n\
  -h, --help            display this help text and exit\n\
  -l, --load            load dungeon file\n\
  -n, --nummon=[NUM]    number of monsters per floor\n\
  -s, --save            save dungeon file\n\
  -z, --seed=[SEED]     set rand seed, takes integer or string");
	}

	exit(status);
}

static void
print_deathscreen(WINDOW *const win)
{
	if (wclear(win) == ERR) {
		cerrx(1, "clear on deathscreen");
	}

	(void)box(win, 0, 0);
	(void)mvwprintw(win, HEIGHT / 2 - 1, WIDTH / 4, "You're dead, Jim.");
	(void)mvwprintw(win, HEIGHT / 2 + 0, WIDTH / 4,
		"\t\t-- McCoy, stardate 3468.1");
	(void)mvwprintw(win, HEIGHT / 2 + 2, WIDTH / 4,
		"You've died. Game over.");
	(void)mvwprintw(win, HEIGHT - 1, 2, "[ Press any key to exit ]");

	if (wrefresh(win) == ERR) {
		cerrx(1, "wrefresh on deathscreen");
	}

	(void)wgetch(win);
}

static void
print_winscreen(WINDOW *const win)
{
	if (wclear(win) == ERR) {
		cerrx(1, "clear on winscreen");
	}

	(void)box(win, 0, 0);
	(void)mvwprintw(win, HEIGHT / 2 - 3, WIDTH / 12,
		"[War] is instinctive. But the insinct can be fought. We're human");
	(void)mvwprintw(win, HEIGHT / 2 - 2, WIDTH / 12,
		"beings with the blood of a million savage years on our hands! But we");
	(void)mvwprintw(win, HEIGHT / 2 - 1, WIDTH / 12,
		"can stop it. We can admit that we're killers ... but we're not going");
	(void)mvwprintw(win, HEIGHT / 2 + 0, WIDTH / 12,
		"to kill today. That's all it takes! Knowing that we're not going to");
	(void)mvwprintw(win, HEIGHT / 2 + 1, WIDTH / 12, "kill today!");
	(void)mvwprintw(win, HEIGHT / 2 + 2, WIDTH / 12,
		"\t\t-- Kirk, \"A Taste of Armageddon\", stardate 3193.0");
	(void)mvwprintw(win, HEIGHT / 2 + 4, WIDTH / 12, "You've won. Game over.");
	(void)mvwprintw(win, HEIGHT - 1, 2, "[ Press any key to exit ]");

	if (wrefresh(win) == ERR) {
		cerrx(1, "wrefresh on winscreen");
	}

	(void)wgetch(win);
}
