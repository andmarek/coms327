#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "cerr.h"
#include "dijk.h"
#include "globs.h"
#include "heap.h"
#include "npc.h"
#include "rand.h"

#define PLAYER_TYPE	0x80
#define INTELLIGENT	0x1
#define TELEPATHIC	0x2
#define TUNNELING	0x4
#define ERRATIC		0x8

#define CUTOFF	4.0 /* minimum distance from the PC an NPC can be placed */

#define TUNNEL_STRENGTH	85
#define PERSISTANCE	5

#define TYPE_MIN	0x0
#define TYPE_MAX	0xF
#define SPEED_MIN	5
#define SPEED_MAX	20

#define KEY_ESC	27

enum pc_action {
	PC_MONSTER_LIST,
	PC_NEXT,
	PC_NONE,
	PC_QUIT
};

static int	valid_npc(int const, int const);

static double		distance(uint8_t const, uint8_t const, uint8_t const, uint8_t const);
static unsigned int	subu32(unsigned int const, unsigned int const);
static bool		pc_visible(struct npc const *restrict const);
static unsigned int	limited_int_to_char(uint8_t const);

static void	move_redraw(WINDOW *const, struct npc *const, uint8_t const, uint8_t const);
static void	move_tunnel(WINDOW *const, struct npc *const, uint8_t const, uint8_t const);

static void	move_straight(WINDOW *const, struct npc *const);
static void	move_dijk_nontunneling(WINDOW *const, struct npc *const);
static void	move_dijk_tunneling(WINDOW *const, struct npc *const);

static void	gen_monster(struct npc *const);

static enum pc_action	turn_npc(WINDOW *const, struct npc *const);
static enum pc_action	turn_pc(WINDOW *const, struct npc *const);
static int32_t		compare_npc(void const *const, void const *const);

static void	monster_list(WINDOW *const, struct npc *monsters, unsigned int const);

struct npc player;

enum turn_exit
turn_engine(WINDOW *const win, unsigned int const nummon)
{
	WINDOW *mwin; /* monster list window */
	struct heap heap;
	struct npc *monsters;
	struct npc *n;
	int32_t turn;
	unsigned int i;
	unsigned int alive = nummon;
	enum turn_exit ret = TURN_NONE;

	monsters = malloc(sizeof(struct npc) * nummon);

	if (monsters == NULL && nummon != 0) {
		cerr(1, "turn_engine malloc");
	}

	heap_init(&heap, compare_npc, NULL);

	player.speed = 10;
	player.type = PLAYER_TYPE;
	player.turn = 0;
	player.type_ch = PLAYER;

	tiles[player.y][player.x].n = &player;

	(void)mvwaddch(win, player.y, player.x, player.type_ch);

	if(heap_insert(&heap, &player) == NULL) {
		cerr(1, "turn_engine heap_insert pc");
	}

	for (i = 0; i < nummon; ++i) {
		gen_monster(&monsters[i]);

		(void)mvwaddch(win, monsters[i].y, monsters[i].x, monsters[i].type_ch);

		if (heap_insert(&heap, &monsters[i]) == NULL) {
			cerr(1, "turn_engine heap_insert npc");
		}
	}

	dijkstra();

	if ((mwin = newwin(HEIGHT, WIDTH, 0, 0)) == NULL) {
		cerrx(1, "turn_engine newwin mwin");
	}

	if (keypad(mwin, true) == ERR) {
		cerrx(1, "keypad mwin");
	}

	while ((n = heap_remove_min(&heap))) {
		if (n->type & PLAYER_TYPE) {
			if (wrefresh(win) == ERR) {
				cerrx(1, "turn_engine wrefresh");
			}
		}

		if (alive == 0) {
			ret = TURN_WIN;
			goto exit;
		}

		if (n->dead) {
			if (n->type & PLAYER_TYPE) {
				ret = TURN_DEATH;
				goto exit;
			} else {
				alive--;
			}
			continue;
		}

		turn = n->turn + 1;
		n->turn = turn + 1000/n->speed;

		retry:
		switch(turn_npc(win, n)) {
		case PC_MONSTER_LIST:
			monster_list(mwin, monsters, nummon);
			touchwin(win);
			wrefresh(win);
			goto retry;
		case PC_NEXT:
			ret = TURN_NEXT;
			goto exit;
		case PC_NONE:
			break;
		case PC_QUIT:
			ret = TURN_QUIT;
			goto exit;
		}

		if (heap_insert(&heap, n) == NULL) {
			cerr(1, "turn_engine re-heap_insert");
		}
	}

	exit:

	delwin(mwin);

	heap_delete(&heap);

	free(monsters);

	return ret;
}

static int
valid_npc(int const y, int const x)
{
	if (tiles[y][x].h != 0) {
		return false;
	}

	return distance(player.x, player.y, (uint8_t)x, (uint8_t)y) > CUTOFF;
}

static double
distance(uint8_t const x0, uint8_t const y0, uint8_t const x1, uint8_t const y1)
{
	int const dx = x1 - x0;
	int const dy = y1 - y0;
	return sqrt(dx * dx + dy * dy);
}

/*
 * From my branchfree saturating arithmetic library.
 * See: https://github.com/esote/bsa
 */
static unsigned int
subu32(unsigned int const a, unsigned int const b)
{
	unsigned int res = a - b;
	res &= (unsigned int) (-(unsigned int) (res <= a));
	return res;
}

/* Bresenham's line algorithm */
static bool
pc_visible(struct npc const *restrict const n)
{
	int x0 = player.x;
	int y0 = player.y;

	int const x1 = n->x;
	int const y1 = n->y;

	int const dx = (uint_fast8_t) abs(x1 - x0);
	int const dy = (uint_fast8_t) abs(y1 - y0);
	int const sx = x0 < x1 ? 1 : -1;
	int const sy = y0 < y1 ? 1 : -1;

	int err = (dx > dy ? dx : -dy) / 2;
	int e2;

	while(1) {
		if (tiles[y0][x0].h != 0) {
			return false;
		}

		if (x0 == x1 && y0 == y1) {
			break;
		}

		e2 = err;

		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}

		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}

	return true;
}

static unsigned int
limited_int_to_char(uint8_t const i)
{
	return (unsigned int)((i < 0xA) ? i + '0' : i + 'a' - 0xA);
}

static void
move_redraw(WINDOW *const win, struct npc *const n, uint8_t const y,
	uint8_t const x)
{
	if (n->x == x && n->y == y) {
		return;
	}

	if (tiles[y][x].n != NULL) {
		(tiles[y][x].n)->dead = true;
	}

	tiles[n->y][n->x].n = NULL;
	tiles[y][x].n = n;

	(void)mvwaddch(win, n->y, n->x, tiles[n->y][n->x].c);
	(void)mvwaddch(win, y, x, n->type_ch);

	n->y = y;
	n->x = x;
}

static void
move_tunnel(WINDOW *const win, struct npc *const n, uint8_t const y,
	uint8_t const x)
{
	if (tiles[y][x].h == UINT8_MAX) {
		return;
	}

	tiles[y][x].h = (uint8_t)subu32(tiles[y][x].h, TUNNEL_STRENGTH);

	dijkstra();

	if (tiles[y][x].h != 0) {
		return;
	}

	if (tiles[y][x].c == ROCK) {
		tiles[y][x].c = CORRIDOR;
	}

	move_redraw(win, n, y, x);
}

static void
move_straight(WINDOW *const win, struct npc *const n)
{
	double min = DBL_MAX;
	double dist;
	int i, j;
	uint8_t x, y;
	uint8_t minx = n->x;
	uint8_t miny = n->y;

	for (i = -1; i <= 1; ++i) {
		for (j = -1; j <= 1; ++j) {
			x = (uint8_t)(n->x + i);
			y = (uint8_t)(n->y + j);

			if (!(n->type & TUNNELING) && tiles[y][x].h != 0) {
				continue;
			}

			dist = distance(player.x, player.y, x, y);

			if (dist < min) {
				min = dist;
				minx = x;
				miny = y;
			}
		}
	}

	if (n->type & TUNNELING) {
		move_tunnel(win, n, miny, minx);
	} else {
		move_redraw(win, n, miny, minx);
	}
}

static void
move_dijk_nontunneling(WINDOW *const win, struct npc *const n)
{
	int32_t min_d = tiles[n->y][n->x].d;
	int i, j;
	uint8_t x, y;
	uint8_t minx = n->x;
	uint8_t miny = n->y;

	for (i = -1; i <= 1; ++i) {
		for (j = -1; j <= 1; ++j) {
			x = (uint8_t)(n->x + i);
			y = (uint8_t)(n->y + j);


			if (tiles[y][x].h != 0) {
				continue;
			}

			if (tiles[y][x].d < min_d) {
				min_d = tiles[y][x].d;
				minx = x;
				miny = y;
			}
		}
	}

	move_redraw(win, n, miny, minx);
}

static void
move_dijk_tunneling(WINDOW *const win, struct npc *const n)
{
	int32_t min_dt = tiles[n->y][n->x].dt;
	int i, j;
	uint8_t x, y;
	uint8_t minx = n->x;
	uint8_t miny = n->y;

	for (i = -1; i <= 1; ++i) {
		for (j = -1; j <= 1; ++j) {
			x = (uint8_t)(n->x + i);
			y = (uint8_t)(n->y + j);

			if (tiles[y][x].dt < min_dt) {
				min_dt = tiles[y][x].dt;
				minx = x;
				miny = y;
			}
		}
	}

	move_tunnel(win, n, miny, minx);
}

static void
gen_monster(struct npc *const n)
{
	int x, y;

	do {
		x = rrand(1, WIDTH - 2);
		y = rrand(1, HEIGHT - 2);
	} while (!valid_npc(y, x));

	*n = (struct npc) {
		.x = (uint8_t)x,
		.y = (uint8_t)y,
		.type = (uint8_t)rrand(TYPE_MIN, TYPE_MAX),
		.speed = (uint8_t)rrand(SPEED_MIN, SPEED_MAX),
		.turn = 0,
		.p_count = 0,
		.dead = false
	};

	n->type_ch = limited_int_to_char(n->type);

	tiles[y][x].n = n;
}

static enum pc_action
turn_npc(WINDOW *const win, struct npc *const n)
{
	uint8_t y, x;

	if (n->type & PLAYER_TYPE) {
		return turn_pc(win, n);
	}

	if (n->type & ERRATIC && rrand(0, 1) == 0) {
		do {
			y = (uint8_t)(n->y + rrand(-1, 1));
			x = (uint8_t)(n->x + rrand(-1, 1));
		} while(!(n->type & TUNNELING) && tiles[y][x].h != 0);

		if (n->type & TUNNELING) {
			move_tunnel(win, n, y, x);
		} else {
			move_redraw(win, n, y, x);
		}

		return PC_NONE;
	}

	switch(n->type) {
	case 0x0:
	case 0x8:
	case 0x4:
	case 0xC:
		/*
		 * straight line if can see player
		 * or tunnel: straight line if can see player
		 */
		if (pc_visible(n)) {
			move_straight(win, n);
		}
		break;
	case 0x2:
	case 0xA:
	case 0x6:
	case 0xE:
		/*
		 * straight line, telepathic towards player
		 * or tunnel: straight line, telepathic towards player
		 */
		move_straight(win, n);
		break;
	case 0x1:
	case 0x9:
	case 0x3:
	case 0xB:
		/*
		 * use nontunneling dijk, remembered location
		 * or use nontunneling dijk, telepathic towards player
		 */
		if (n->type & TELEPATHIC || pc_visible(n)) {
			n->p_count = PERSISTANCE;
		}

		if (n->p_count != 0) {
			move_dijk_nontunneling(win, n);
			n->p_count--;
		}
		break;
	case 0x5:
	case 0xD:
	case 0x7:
	case 0xF:
		/*
		 * tunnel: use tunneling dijk, remembered location
		 * or tunnel: use tunneling dijk, telepathic towards player
		 */
		if (n->type & TELEPATHIC || pc_visible(n)) {
			n->p_count = PERSISTANCE;
		}

		if (n->p_count != 0) {
			move_dijk_tunneling(win, n);
			n->p_count--;
		}
		break;
	default:
		cerrx(1, "gen_monster invalid monster type %d", n->type);
	}

	return PC_NONE;
}

static enum pc_action
turn_pc(WINDOW *const win, struct npc *const n)
{
	uint8_t y = n->y;
	uint8_t x = n->x;
	bool exit = false;

	while(!exit) {
		exit = true;
		switch(wgetch(win)) {
		case ERR:
			cerrx(1, "turn_pc wgetch ERR");
			break;
		case KEY_HOME:
		case KEY_A1:
		case '7':
		case 'y':
			/* up left */
			y--;
			x--;
			break;
		case KEY_UP:
		case '8':
		case 'k':
			/* up */
			y--;
			break;
		case KEY_PPAGE:
		case KEY_A3:
		case '9':
		case 'u':
			/* up right */
			y--;
			x++;
			break;
		case KEY_RIGHT:
		case '6':
		case 'l':
			/* right */
			x++;
			break;
		case KEY_NPAGE:
		case KEY_C3:
		case '3':
		case 'n':
			/* down right */
			y++;
			x++;
			break;
		case KEY_DOWN:
		case '2':
		case 'j':
			/* down */
			y++;
			break;
		case KEY_END:
		case KEY_C1:
		case '1':
		case 'b':
			/* down left */
			y++;
			x--;
			break;
		case KEY_LEFT:
		case '4':
		case 'h':
			/* left */
			x--;
			break;
		case KEY_B2:
		case ' ':
		case '5':
		case '.':
			/* rest */
			return PC_NONE;
		case '>':
			/* go down stairs */
			if (tiles[y][x].c == STAIR_DN) {
				return PC_NEXT;
			} else {
				exit = false;
			}
			break;
		case '<':
			/* go up stairs */
			if (tiles[y][x].c == STAIR_UP) {
				return PC_NEXT;
			} else {
				exit = false;
			}
			break;
		case 'm':
			return PC_MONSTER_LIST;
		case 'Q':
		case 'q':
			/* quit game */
			return PC_QUIT;
		default:
			exit = false;
		}
	}

	if (tiles[y][x].h == 0) {
		move_redraw(win, n, y, x);
		dijkstra();
	}

	return PC_NONE;
}

static int32_t
compare_npc(void const *const key, void const *const with)
{
	return ((struct npc const *const) key)->turn - ((struct npc const *const) with)->turn;
}

static void
monster_list(WINDOW *const mwin, struct npc *monsters, unsigned int const nummon)
{
	struct npc n;
	int dx;
	int dy;
	unsigned int i;
	unsigned int cpos = 0;

	while(1) {
		wclear(mwin);
		(void)box(mwin, 0, 0);

		for (i = 0; i < HEIGHT - 2 && i + cpos < nummon; ++i) {
			n = monsters[i + cpos];
			dx = player.x - n.x;
			dy = player.y - n.y;

			(void)mvwprintw(mwin, (int)(i + 1U), 2,
				"%u.\t%c, %d %s and %d %s", i + cpos, n.type_ch,
				abs(dy), dy > 0 ? "north" : "south",
				abs(dx), dx > 0 ? "east" : "west");
		}

		for (; i < HEIGHT - 2; ++i) {
			(void)mvwaddch(mwin, (int)(i + 1U), 2, '~');
		}

		wrefresh(mwin);

		switch(wgetch(mwin)) {
		case ERR:
			cerrx(1, "monster_list wgetch ERR");
			return;
		case KEY_UP:
			cpos = subu32(cpos, 1U);
			break;
		case KEY_DOWN:
			if (++cpos > nummon - 1) {
				cpos = nummon - 1;
			}
			break;
		case 27:
			return;
		default:
			break;
		}

	}
}
