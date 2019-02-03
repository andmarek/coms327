#include <stdlib.h>

#include "rand.h"
#include "room.h"

#define MINROOMH	4
#define MINROOMW	3
#define MAXROOMH	8
#define MAXROOMW	15

#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))

void
gen_room(struct room *const r, int w, int h)
{
	r->x = (uint8_t)rrand(1, w - 2);
	r->y = (uint8_t)rrand(1, h - 2);
	r->size_x = (uint8_t)rrand(MINROOMW, MAXROOMW);
	r->size_y = (uint8_t)rrand(MINROOMH, MAXROOMH);
}

int
draw_room(WINDOW *const win, struct room const *const r)
{
	int i, j;

	for (i = r->x - 1; i <= r->x + r->size_x + 1; ++i) {
		for (j = r->y - 1; j <= r->y + r->size_y + 1; ++j) {
			if (mvwinch(win, j, i) != ROCK) {
				return false;
			}
		}
	}

	for (i = r->x; i <= r->x + r->size_x; ++i) {
		for (j = r->y; j <= r->y + r->size_y; ++j) {
			mvwaddch(win, j, i, ROOM);
		}
	}

	return true;
}

static int
valid_corridor_x(WINDOW *const win, int x, int y)
{
	return mvwinch(win, y, x) == ROCK
		&& mvwinch(win, y, x + 1) != CORRIDOR
		&& mvwinch(win, y, x - 1) != CORRIDOR;
}

static int
valid_corridor_y(WINDOW *const win, int x, int y)
{
	return mvwinch(win, y, x) == ROCK
		&& mvwinch(win, y + 1, x) != CORRIDOR
		&& mvwinch(win, y - 1, x) != CORRIDOR;
}

void
draw_corridor(WINDOW *const win, struct room const r1, struct room const r2)
{
	int i;

	for (i = MIN(r1.x, r2.x); i <= MAX(r1.x, r2.x); ++i) {
		if (valid_corridor_y(win, i, r1.y)) {
			mvwaddch(win, r1.y, i, CORRIDOR);
		}
	}

	for (i = MIN(r1.y, r2.y); i <= MAX(r1.y, r2.y); ++i) {
		if (valid_corridor_x(win, r2.x, i)) {
			mvwaddch(win, i, r2.x, CORRIDOR);
		}
	}
}

static int
valid_stair(WINDOW *const win, int x, int y)
{
	return (mvwinch(win, y, x) == ROCK || mvwinch(win, y, x) == ROOM)
		&& (mvwinch(win, y + 1, x) == CORRIDOR
		|| mvwinch(win, y - 1, x) == CORRIDOR
		|| mvwinch(win, y, x + 1) == CORRIDOR
		|| mvwinch(win, y, x - 1) == CORRIDOR);
}

void
draw_stair(WINDOW *const win, int w, int h, int up)
{
	int x, y;

	do {
		x = rrand(1, w - 2);
		y = rrand(1, h - 2);
	} while(!valid_stair(win, x, y));

	mvwaddch(win, y, x, up ? STAIR_UP : STAIR_DOWN);
}
