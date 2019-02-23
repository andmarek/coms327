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

	(void)mvwaddch(win, y, x, PLAYER);

	p.x = (uint8_t)x;
	p.y = (uint8_t)y;
}

int move_player(WINDOW *const win, uint8_t const y, uint8_t const x)
{
	if (tiles[y][x].h != 0) {
		return -1;
	}

	(void)mvwaddch(win, p.y, p.x, tiles[p.y][p.x].c);

	p.y = y;
	p.x = x;

	(void)mvwaddch(win, p.y, p.x, PLAYER);

	return 0;
}
