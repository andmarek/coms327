#include "floor.h"
#include "player.h"
#include "rand.h"

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

	mvwaddch(win, y, x, PLAYER);
}
