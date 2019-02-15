#include <getopt.h>
#include <limits.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

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

int const width = 80;
int const height = 21;

struct tile *tiles;

static void	usage(int const, char const *const);
static int	register_tiles(WINDOW *const, int const, int const);
static int	arrange_new(WINDOW *const, int const, int const);
static void	arrange_loaded(WINDOW *const, int const, int const);
static int	gen(void);
static int32_t	compare(void const *, void const *);
static void	calc_cost(struct heap *, struct heap_node **, struct tile const *const, int const);
static int	dijstra(int const, int const, int const);
static void	print_nontunneling(int const, int const);
static void	print_tunneling(int const, int const);

int
main(int const argc, char *const argv[])
{
	WINDOW *win;
	int ch, ok = EXIT_SUCCESS;
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

	if (dijstra(width, height, false) == -1) {
		fprintf(stderr, "error in non-tunneling dijstra\n");
		ok = EXIT_FAILURE;
		goto exit;
	}
	print_nontunneling(width, height);

	if (dijstra(width, height, true) == -1) {
		fprintf(stderr, "error in tunneling dijstra\n");
		ok = EXIT_FAILURE;
		goto exit;
	}
	print_tunneling(width, height);

	exit:

	if (win != NULL) {
		delwin(win);
		endwin();
	}

	/* zero memory before freeing */
	memset(rooms, 0, sizeof(struct room) * room_count);
	memset(stairs_up, 0, sizeof(struct stair) * stair_up_count);
	memset(stairs_dn, 0, sizeof(struct stair) * stair_dn_count);
	memset(tiles, 0, sizeof(struct tile) * (size_t)width * (size_t)height);

	free(rooms);
	free(stairs_up);
	free(stairs_dn);
	free(tiles);

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

	for (i = 0; i < w; ++i) {
		for (j = 0; j < h; ++j) {
			tiles[j * w + i].c = mvwinch(win, j, i);

			if (i == 0 || j == 0 || i == w - 1 || j == h - 1) {
				tiles[j * w + i].h = UINT8_MAX;
				continue;
			}

			switch(tiles[j * w + i].c) {
			case ROOM:
			case CORRIDOR:
			case STAIR_UP:
			case STAIR_DN:
				tiles[j * w + i].h = 0;
				break;
			case PLAYER:
				/* call before player is placed */
				return -1;
			default:
				tiles[j * w + i].h = (uint8_t)rrand(1, UINT8_MAX - 1U);
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

	for (i = 1; i < w - 1; ++i) {
		for (j = 1; j < h - 1; ++j) {
			ch = mvwinch(win, j, i);

			if (tiles[j * w + i].h == 0 && ch == ROCK) {
				mvwaddch(win, j, i, CORRIDOR);
				ch = mvwinch(win, j, i);
			}

			tiles[j * w + i].c = ch;
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

	if (!(tiles = malloc(sizeof(struct tile) * (size_t)width * (size_t)height))) {
		return -1;
	}

	return 0;
}

static int32_t
compare(void const *const key, void const *const with)
{
	return ((struct tile const *const) key)->d - ((struct tile const *const) with)->d;
}

static void
calc_cost(struct heap *const h, struct heap_node **nodes,
	struct tile const *const t, int const index)
{
	if (nodes[index] != NULL && tiles[index].d > t->d + t->h/85) {
		tiles[index].d = t->d + 1 + t->h/85;
		heap_decrease_key_no_replace(h, nodes[index]);
	}
}

static int
dijstra(int const w, int const h, int const tunnel)
{
	struct heap_node **nodes;
	struct heap heap;
	struct tile *t;
	int i, j;

	nodes = malloc(sizeof(struct heap_node) * (size_t)w * (size_t)h);

	if (nodes == NULL) {
		return -1;
	}

	heap_init(&heap, compare, NULL);

	tiles[p.y * w + p.x].d = 0;

	for (i = 0; i < w; ++i) {
		for (j = 0; j < h; ++j) {
			if (i != p.x || j != p.y) {
				tiles[j * w + i].d = INT32_MAX;
			}

			tiles[j * w + i].x = (uint8_t)i;
			tiles[j * w + i].y = (uint8_t)j;

			if ((tunnel && tiles[j * w + i].h == UINT8_MAX)
				|| (!tunnel && tiles[j * w + i].h != 0)) {
				nodes[j * w + i] = NULL;
			} else {
				nodes[j * w + i] = heap_insert(&heap, &tiles[j * w + i]);
			}
		}
	}

	while((t = heap_remove_min(&heap))) {
		calc_cost(&heap, nodes, t, (t->y - 1) * w + (t->x + 0));
		calc_cost(&heap, nodes, t, (t->y + 1) * w + (t->x + 0));

		calc_cost(&heap, nodes, t, (t->y + 0) * w + (t->x - 1));
		calc_cost(&heap, nodes, t, (t->y + 0) * w + (t->x + 1));

		calc_cost(&heap, nodes, t, (t->y + 1) * w + (t->x + 1));
		calc_cost(&heap, nodes, t, (t->y - 1) * w + (t->x - 1));

		calc_cost(&heap, nodes, t, (t->y - 1) * w + (t->x + 1));
		calc_cost(&heap, nodes, t, (t->y + 1) * w + (t->x - 1));
	}

	free(nodes);

	heap_delete(&heap);

	return 0;
}

static void
print_nontunneling(int const w, int const h)
{
	int i, j;

	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
			if (i == p.y && j == p.x) {
				putchar('@');
			} else if (tiles[i * w + j].h == 0) {
				printf("%d", tiles[i * w + j].d % 10);
			} else {
				putchar(' ');
			}
		}
		putchar('\n');
	}
}

static void
print_tunneling(int const w, int const h)
{
	int i, j;

	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
			if (i == 0 || j == 0 || i == h - 1 || j == w - 1) {
				putchar('X');
			} else if (i == p.y && j == p.x) {
				putchar('@');
			} else {
				printf("%d", tiles[i * w + j].d % 10);
			}
		}
		putchar('\n');
	}
}
