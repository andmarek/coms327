#include "player.h"
#include "rand.h"
#include "room.h"

static int
valid_player(WINDOW *const win, int x, int y)
{
	return mvwinch(win, y, x) == ROOM
		&& mvwinch(win, y + 1, x) == ROOM
		&& mvwinch(win, y - 1, x) == ROOM
		&& mvwinch(win, y, x + 1) == ROOM
		&& mvwinch(win, y, x - 1) == ROOM;
}

void
place_player(WINDOW *const win, int w, int h)
{
	int x, y;

	do {
		x = rrand(1, w - 2);
		y = rrand(1, h - 2);
	} while (!valid_player(win, x, y));

	mvwaddch(win, y, x, PLAYER);
}
