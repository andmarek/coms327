#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <thread>

#include <getopt.h>

#include "cerr.h"
#include "gen.h"
#include "globs.h"
#include "npc.h"
#include "parse.h"

static void	usage(int const, std::string const &);

static void	print_deathscreen(WINDOW *const);
static void	print_winscreen(WINDOW *const);

static bool	is_number(std::string const &);

static char const *const PROGRAM_NAME = "opal";

static struct option const long_opts[] = {
	{"nodescs", no_argument, NULL, 'd'},
	{"help", no_argument, NULL, 'h'},
	{"load", no_argument, NULL, 'l'},
	{"nummon", required_argument, NULL, 'n'},
	{"save", no_argument, NULL, 's'},
	{"seed", required_argument, NULL, 'z'},
	{NULL, 0, NULL, 0}
};

int
main(int const argc, char *const argv[])
{
	WINDOW *win;
	char *end;
	int ch;
	bool load = false;
	bool save = false;
	bool no_descs = false;
	unsigned int nummon = std::numeric_limits<unsigned int>::max();
	std::string const name = (argc == 0) ? PROGRAM_NAME : argv[0];

	while ((ch = getopt_long(argc, argv, "dhln:sz:", long_opts, NULL)) != -1) {
		switch(ch) {
		case 'd':
			no_descs = true;
			break;
		case 'h':
			usage(EXIT_SUCCESS, name);
			break;
		case 'l':
			load = true;
			break;
		case 'n':
			nummon = (unsigned int)strtoul(optarg, &end, 10);

			if (optarg == end || errno == EINVAL || errno == ERANGE) {
				cerr(1, "nummon invalid");
			}
			break;
		case 's':
			save = true;
			break;
		case 'z':
			if (is_number(optarg)) {
				rr = ranged_random(strtoul(optarg, &end, 10));

				if (optarg == end || errno == EINVAL
					|| errno == ERANGE) {
					cerr(1, "seed %s invalid", optarg);
				}
			} else {
				rr = ranged_random(optarg);
			}
			break;
		default:
			usage(EXIT_FAILURE, name);
		}
	}

	if (!no_descs) {
		parse_npc_file();
		parse_obj_file();
	}

	if (nummon == std::numeric_limits<unsigned int>::max()) {
		nummon = rr.rrand<unsigned int>(3, 5);
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
		if (!load_dungeon()) {
			cerrx(1, "loading dungeon");
		}

		arrange_loaded();
	} else {
		arrange_new();
	}

	retry:
	switch(turn_engine(win, nummon)) {
	case TURN_DEATH:
		std::this_thread::sleep_for(std::chrono::seconds(1));

		print_deathscreen(win);

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		break;
	case TURN_NEXT:
		if (wclear(win) == ERR) {
			cerrx(1, "arrange_renew clear");
		}

		(void)box(win, 0, 0);

		arrange_renew();

		goto retry;

		break;
	case TURN_NONE:
		cerrx(1, "turn_engine return value invalid");
		break;
	case TURN_QUIT:
		break;
	case TURN_WIN:
		std::this_thread::sleep_for(std::chrono::seconds(1));

		print_winscreen(win);

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		break;
	}

	if (delwin(win) == ERR) {
		cerrx(1, "delwin");
	}

	if (endwin() == ERR) {
		cerrx(1, "endwin");
	}

	std::cout << "seed: " << rr.seed << '\n';

	if (save && !save_dungeon()) {
		cerrx(1, "saving dungeon");
	}

	return EXIT_SUCCESS;
}

static void
usage(int const status, std::string const &name)
{
	std::cout << "Usage: " << name << " [OPTION]... \n\n";

	if (status != EXIT_SUCCESS) {
		std::cerr << "Try '" << name <<
			" --help' for more information.\n";
	} else {
		std::cout << "OPAL's Playable Almost Indefectibly.\n\n"
			<< "Traverse a generated dungeon.\n\n"
			<< "Options:\n\
  -d, --nodescs         don't parse description files\n\
  -h, --help            display this help text and exit\n\
  -l, --load            load dungeon file\n\
  -n, --nummon=[NUM]    number of monsters per floor\n\
  -s, --save            save dungeon file\n\
  -z, --seed=[SEED]     set rand seed, takes integer or string\n";
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
	(void)mvwprintw(win, HEIGHT - 1, 2, "[ press any key to exit ]");

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
	(void)mvwprintw(win, HEIGHT - 1, 2, "[ press any key to exit ]");

	if (wrefresh(win) == ERR) {
		cerrx(1, "wrefresh on winscreen");
	}

	(void)wgetch(win);
}

bool
is_number(std::string const &s)
{
	return !s.empty() && std::find_if(s.begin(), s.end(), [](char const c) {
		return !std::isdigit(c);
	}) == s.end();
}
