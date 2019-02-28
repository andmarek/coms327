#include "globs.h"
#include "rand.h"

#define MINROOMH	4
#define MINROOMW	3
#define MAXROOMH	8
#define MAXROOMW	15

#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))

void
gen_room(struct room *const r)
{
	r->x = (uint8_t)rrand(1, WIDTH - 2);
	r->y = (uint8_t)rrand(1, HEIGHT - 2);
	r->size_x = (uint8_t)rrand(MINROOMW, MAXROOMW);
	r->size_y = (uint8_t)rrand(MINROOMH, MAXROOMH);
}

int
valid_room(struct room const *const r)
{
	int i, j;

	for (i = r->x - 1; i <= r->x + r->size_x + 1; ++i) {
		for (j = r->y - 1; j <= r->y + r->size_y + 1; ++j) {
			if (tiles[j][i].c != ROCK) {
				return -1;
			}
		}
	}

	return 0;
}

void
draw_room(WINDOW *const win, struct room const *const r)
{
	int i, j;

	for (i = r->x; i < r->x + r->size_x; ++i) {
		for (j = r->y; j < r->y + r->size_y; ++j) {
			tiles[j][i].c = ROOM;
			(void)mvwaddch(win, j, i, ROOM);
		}
	}
}

static int
valid_corridor_x(int const y, int const x)
{
	return tiles[y][x].c == ROCK
		&& tiles[y][x + 1].c != CORRIDOR
		&& tiles[y][x - 1].c != CORRIDOR;
}

static int
valid_corridor_y(int const y, int const x)
{
	return tiles[y][x].c == ROCK
		&& tiles[y + 1][x].c != CORRIDOR
		&& tiles[y - 1][x].c != CORRIDOR;
}

void
draw_corridor(WINDOW *const win, struct room const r1, struct room const r2)
{
	int i;

	for (i = MIN(r1.x, r2.x); i <= MAX(r1.x, r2.x); ++i) {
		if (valid_corridor_y(r1.y, i)) {
			tiles[r1.y][i].c = CORRIDOR;
			(void)mvwaddch(win, r1.y, i, CORRIDOR);
		}
	}

	for (i = MIN(r1.y, r2.y); i <= MAX(r1.y, r2.y); ++i) {
		if (valid_corridor_x(i, r2.x)) {
			tiles[i][r2.x].c = CORRIDOR;
			(void)mvwaddch(win, i, r2.x, CORRIDOR);
		}
	}
}

static int
valid_stair(int const y, int const x)
{
	return (tiles[y][x].c == ROCK || tiles[y][x].c == ROOM)
		&& (tiles[y + 1][x].c == CORRIDOR
		|| tiles[y - 1][x].c == CORRIDOR
		|| tiles[y][x + 1].c == CORRIDOR
		|| tiles[y][x - 1].c == CORRIDOR);
}

void
gen_draw_stair(WINDOW *const win, struct stair *const s, bool const up)
{
	int x, y;

	do {
		x = rrand(1, WIDTH - 2);
		y = rrand(1, HEIGHT - 2);
	} while(!valid_stair(y, x));

	tiles[y][x].c = up ? STAIR_UP : STAIR_DN;
	(void)mvwaddch(win, y, x, up ? STAIR_UP : STAIR_DN);

	s->x = (uint8_t)x;
	s->y = (uint8_t)y;
}
