#include <stdlib.h>

#include "rand.h"
#include "room.h"

#define MINROOMH	4
#define MINROOMW	3
#define MAXROOMH	8
#define MAXROOMW	15

void
gen_room(struct room *const r, int w, int h)
{
	r->x = rrand(1, w - 2);
	r->y = rrand(1, h - 2);
	r->size_x = rrand(MINROOMW, MAXROOMW);
	r->size_y = rrand(MINROOMH, MAXROOMH);
}

bool
draw_room(WINDOW *const win, struct room const r)
{
	int i, j;

	for (i = r.x - 1; i <= r.x + r.size_x + 1; ++i) {
		for (j = r.y - 1; j <= r.y + r.size_y + 1; ++j) {
			if (mvwinch(win, j, i) != ROCK) {
				return false;
			}
		}
	}

	for (i = r.x; i <= r.x + r.size_x; ++i) {
		for (j = r.y; j <= r.y + r.size_y; ++j) {
			mvwaddch(win, j, i, ROOM);
		}
	}

	return true;
}

void
draw_corridor(WINDOW *const win, struct room const r1, struct room const r2)
{
	int i;

	int const distance = r2.x - r1.x / (int)abs(r2.x - r1.x);

	for (i = r1.x; i <= r2.x - r1.x; ++i) {
		mvwaddch(win, r1.y, i, CORRIDOR);
	}
}
