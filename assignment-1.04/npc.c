#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "cerr.h"
#include "dijk.h"
#include "floor.h"
#include "heap.h"
#include "npc.h"
#include "opal.h"
#include "rand.h"

#define PLAYER_TYPE	0x80
#define INTELLIGENT	0x1
#define TELEPATHIC	0x2
#define TUNNELING	0x4
#define ERRATIC		0x8

static int
valid_player(WINDOW *const win, int const y, int const x)
{
	return mvwinch(win, y, x) == ROOM
		&& mvwinch(win, y + 1, x) == ROOM
		&& mvwinch(win, y - 1, x) == ROOM
		&& mvwinch(win, y, x + 1) == ROOM
		&& mvwinch(win, y, x - 1) == ROOM;
}

void
place_player(WINDOW *const win, int const w, int const h)
{
	int x, y;

	do {
		x = rrand(1, w - 2);
		y = rrand(1, h - 2);
	} while (!valid_player(win, y, x));

	(void)mvwaddch(win, y, x, PLAYER);

	player.x = (uint8_t)x;
	player.y = (uint8_t)y;
}

static double
distance(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	int dx = x1 - x0;
	int dy = y1 - y0;
	return sqrt(dx * dx + dy * dy);
}

static int
valid_npc(int const y, int const x)
{
	if (tiles[y][x].c != ROOM) {
		return false;
	}

// TODO move up
#define CUTOFF	4.0
	return distance(player.x, player.y, (uint8_t)x, (uint8_t)y) > CUTOFF;
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

	unsigned int const ch = mvwinch(win, n->y, n->x);
	(void)mvwaddch(win, n->y, n->x, tiles[n->y][n->x].c);
	n->y = y;
	n->x = x;
	(void)mvwaddch(win, y, x, ch);
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

static void
move_tunnel(WINDOW *const win, struct npc *const n, uint8_t const y,
	uint8_t const x)
{
	if (tiles[y][x].h == UINT8_MAX) {
		return;
	}

	tiles[y][x].h = subu8(tiles[y][x].h, 85);

	dijkstra(WIDTH, HEIGHT, player.y, player.x);

	if (tiles[y][x].h != 0) {
		return;
	}

	if (tiles[y][x].c == ROCK) {
		tiles[y][x].c = CORRIDOR;
	}

	move_redraw(win, n, y, x);
}

static void
move_straight(WINDOW *const win, struct npc *const m)
{
	uint8_t x, y;
	uint8_t minx = m->x, miny = m->y;
	int i, j;
	double dist;
	double min = DBL_MAX;

	// TODO unroll?
	for (i = -1; i < 1; ++i) {
		for (j = -1; j < 1; ++j) {
			x = (uint8_t)(m->x + i);
			y = (uint8_t)(m->y + j);

			if (!(m->type & TUNNELING) && tiles[y][x].h != 0) {
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

	if (m->type & TUNNELING) {
		move_tunnel(win, m, miny, minx);
	} else {
		move_redraw(win, m, miny, minx);
	}
}

static void
move_dijk_nontunneling(WINDOW *const win, struct npc *const n)
{
	int32_t min_d = tiles[miny][minx].d;
	int i, j;
	uint8_t x, y;
	uint8_t minx = n->x
	uint8_t miny = n->y;

	// TODO unroll?
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
	int32_t min_dt = tiles[miny][minx].dt;
	int i, j;
	uint8_t x, y;
	uint8_t minx = n->x;
	uint8_t miny = n->y;

	// TODO unroll?
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

static unsigned int
gen_monster(struct npc *const m, int const w, int const h)
{
#define TYPE_MIN	0
#define TYPE_MAX	0xF
#define SPEED_MIN	5
#define SPEED_MAX	20

	int x, y;

	do {
		x = rrand(1, w - 2);
		y = rrand(1, h - 2);
	} while (!valid_npc(y, x));

	m->x = (uint8_t)x;
	m->y = (uint8_t)y;

	tiles[y][x].n = m;

	m->type = 7;//(uint8_t)rrand(TYPE_MIN, TYPE_MAX);
	m->speed = (uint8_t)rrand(SPEED_MIN, SPEED_MAX);
	m->turn = 0;
	m->saw_pc = false;
	m->dead = false;

	switch (m->type) {
	case 0x0:
		return '0';
	case 0x1:
		return '1';
	case 0x2:
		return '2';
	case 0x3:
		return '3';
	case 0x4:
		return '4';
	case 0x5:
		return '5';
	case 0x6:
		return '6';
	case 0x7:
		return '7';
	case 0x8:
		return '8';
	case 0x9:
		return '9';
	case 0xA:
		return 'a';
	case 0xB:
		return 'b';
	case 0xC:
		return 'c';
	case 0xD:
		return 'd';
	case 0xE:
		return 'e';
	case 0xF:
		return 'f';
	default:
		cerrx(1, "gen_monster invalid monster type %d", m->type);
		return 0;
	}
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
void
move_npc(WINDOW *const win, struct npc *const n, int const w, int const h)
{
	uint8_t y, x;

	if (n->type & PLAYER_TYPE) {
		y = (uint8_t)(n->y + rrand(-1, 1));
		x = (uint8_t)(n->x + rrand(-1, 1));

		if (tiles[y][x].h != 0) {
			return;
		}

		move_redraw(win, n, y, x);

		dijkstra(w, h, player.y, player.x);

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
			n->saw_pc = true;
			n->target_x = player.x;
			n->target_y = player.y;
		}

		if (n->saw_pc) {
			move_dijk_nontunneling(win, n);
		}
		break;
	case 0x5:
	case 0xD:
	case 0x7:
	case 0xF:
		// tunnel: use tunneling dijk, remembered location
		// or tunnel: use tunneling dijk, telepathic towards player
		if (n->type & TELEPATHIC || pc_visible(n)) {
			n->saw_pc = true;
			n->target_x = player.x;
			n->target_y = player.y;
		}

		if (n->saw_pc) {
			move_dijk_tunneling(win, n);
		}
		break;
	default:
		cerrx(1, "gen_monster invalid monster type %d", n->type);
	}
}

static int32_t
compare_npc(void const *const key, void const *const with)
{
	return ((struct npc const *const) key)->turn - ((struct npc const *const) with)->turn;
}

void
turn_engine(WINDOW *const win, unsigned int const nummon, int const w, int const h)
{
	struct heap heap;
	struct npc *monsters;
	struct npc *n;
	int32_t turn;
	unsigned int i, ch;
	unsigned int alive = nummon;

	monsters = malloc(sizeof(struct npc) * nummon);

	if (monsters == NULL) {
		cerr(1, "turn_engine malloc");
	}

	heap_init(&heap, compare_npc, NULL);

	player.speed = 10;
	player.type = PLAYER_TYPE;
	player.turn = 0;
	heap_insert(&heap, &player);

	for (i = 0; i < nummon; ++i) {
		ch = gen_monster(&monsters[i], w, h);

		// TODO check != -1
		mvwaddch(win, monsters[i].y, monsters[i].x, ch);

		// TODO error check
		heap_insert(&heap, &monsters[i]);
	}

	wrefresh(win);
	getch();

	while ((n = heap_remove_min(&heap))) {
		if (alive == 0) {
			mvwprintw(win, 0, 2, "[no monsters left]");
			wrefresh(win);
			getch();
			break;
		}
		if (n->dead) {
			mvwprintw(win, 0, 2, "[DIED: n->type: %d, x: %d, y: %d]", n->type, n->x, n->y);
			wrefresh(win);
			getch();
			if (n->type & PLAYER_TYPE) {
				// game over
				break;
			} else {
				alive--;
			}
			continue;
		}

		turn = n->turn + 1;
		n->turn = turn + 1000/n->speed;

		mvwprintw(win, 0, 2, "[n->type: %d, x: %d, y: %d]", n->type, n->x, n->y);
		wrefresh(win);

		move_npc(win, n, w, h);

		if (wrefresh(win) == ERR) {
			cerrx(1, "turn_engine wrefresh");
		}

		// TODO error check
		heap_insert(&heap, n);
	}

	heap_delete(&heap);

	free(monsters);
}
