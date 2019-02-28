#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "cerr.h"
#include "dijk.h"
#include "globs.h"
#include "heap.h"
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

static int	valid_player(int const, int const);
static int	valid_npc(int const, int const);

static double		distance(uint8_t const, uint8_t const, uint8_t const, uint8_t const);
static uint8_t		subu8(unsigned int const, unsigned int const);
static bool		pc_visible(struct npc const *restrict const);
static unsigned int	limited_int_to_char(uint8_t const);


static void	move_redraw(WINDOW *const, struct npc *const, uint8_t const, uint8_t const);
static void	move_tunnel(WINDOW *const, struct npc *const, uint8_t const, uint8_t const);

static void	move_straight(WINDOW *const, struct npc *const);
static void	move_dijk_nontunneling(WINDOW *const, struct npc *const);
static void	move_dijk_tunneling(WINDOW *const, struct npc *const);

static void	gen_monster(struct npc *const);

static void	turn_npc(WINDOW *const, struct npc *const);
static void	turn_pc(WINDOW *const, struct npc *const);
static int32_t	compare_npc(void const *const, void const *const);

static void	print_deathscreen(WINDOW *const);
static void	print_winscreen(WINDOW *const);

struct npc player;

void
place_player(void)
{
	int x, y;

	do {
		x = rrand(1, WIDTH - 2);
		y = rrand(1, HEIGHT - 2);
	} while (!valid_player(y, x));

	player.x = (uint8_t)x;
	player.y = (uint8_t)y;
}

void
turn_engine(WINDOW *const win, unsigned int const nummon)
{
	struct heap heap;
	struct npc *monsters;
	struct npc *n;
	int32_t turn;
	unsigned int i;
	unsigned int alive = nummon;

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

	while ((n = heap_remove_min(&heap))) {
		if (n->type & PLAYER_TYPE) {
			if (wrefresh(win) == ERR) {
				cerrx(1, "turn_engine wrefresh");
			}
		}

		if (alive == 0) {
			sleep(1);
			print_winscreen(win);
			break;
		}

		if (n->dead) {
			if (n->type & PLAYER_TYPE) {
				sleep(1);
				print_deathscreen(win);
				break;
			} else {
				alive--;
			}
			continue;
		}

		turn = n->turn + 1;
		n->turn = turn + 1000/n->speed;

		turn_npc(win, n);

		if (heap_insert(&heap, n) == NULL) {
			cerr(1, "turn_engine re-heap_insert");
		}
	}

	heap_delete(&heap);

	free(monsters);
}

static int
valid_player(int const y, int const x)
{
	return tiles[y][x].h == 0
		&& tiles[y + 1][x].h == 0 && tiles[y - 1][x].h == 0
		&& tiles[y][x + 1].h == 0 && tiles[y][x - 1].h == 0;
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
 * Adapted from my branchfree saturating arithmetic library.
 * See: https://github.com/esote/bsa
 * Preconditon: a and b are within range of uint8_t.
 */
static uint8_t
subu8(unsigned int const a, unsigned int const b)
{
	unsigned int res = a - b;
	res &= (unsigned int) (-(unsigned int) (res <= a));
	return (uint8_t)res;
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

	tiles[y][x].h = subu8(tiles[y][x].h, TUNNEL_STRENGTH);

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

static void
turn_npc(WINDOW *const win, struct npc *const n)
{
	uint8_t y, x;

	if (n->type & PLAYER_TYPE) {
		turn_pc(win, n);
		return;
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

		return;
	}

	switch(n->type) {
	case 0x0:
	case 0x8:
	case 0x4:
	case 0xC:
		// straight line if can see player
		// or tunnel: straight line if can see player
		if (pc_visible(n)) {
			move_straight(win, n);
		}
		break;
	case 0x2:
	case 0xA:
	case 0x6:
	case 0xE:
		// straight line, telepathic towards player
		// or tunnel: straight line, telepathic towards player
		move_straight(win, n);
		break;
	case 0x1:
	case 0x9:
	case 0x3:
	case 0xB:
		// use nontunneling dijk, remembered location
		// or use nontunneling dijk, telepathic towards player
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
		// tunnel: use tunneling dijk, remembered location
		// or tunnel: use tunneling dijk, telepathic towards player
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
}

static void
turn_pc(WINDOW *const win, struct npc *const n)
{
	uint8_t y = n->y;
	uint8_t x = n->x;

	switch(getch()) {
	case KEY_A1:
	case '7':
	case 'y':
		// up left
		y--;
		x--;
		break;
	case KEY_UP:
	case '8':
	case 'k':
		// TODO: scroll up monster list
		// up
		y--;
		break;
	case KEY_A3:
	case '9':
	case 'u':
		// up right
		y--;
		x++;
		break;
	case KEY_RIGHT:
	case '6':
	case 'l':
		// right
		x++;
		break;
	case KEY_C3:
	case '3':
	case 'n':
		// down right
		y++;
		x++;
		break;
	case KEY_DOWN:
	case '2':
	case 'j':
		// TODO: scroll down monster list
		// down
		y++;
		break;
	case KEY_C1:
	case '1':
	case 'b':
		// down left
		y++;
		x--;
		break;
	case KEY_LEFT:
	case '4':
	case 'h':
		// left
		x--;
		break;
	case ' ':
	case '5':
	case '.':
		// rest
		return;
	case '>':
		// TODO
		// go down stairs
		break;
	case '<':
		// TODO
		// go up stairs
		break;
	case 'm':
		// TODO
		// monster list
		break;
	}

	if (tiles[y][x].h != 0) {
		return;
	}

	move_redraw(win, n, y, x);

	dijkstra();
}

static int32_t
compare_npc(void const *const key, void const *const with)
{
	return ((struct npc const *const) key)->turn - ((struct npc const *const) with)->turn;
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
}
