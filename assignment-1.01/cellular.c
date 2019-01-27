/*
#define FILL	45	// fill percentage

void
fill_floor(WINDOW *win)
{
	if (rrand(0, 100) < FILL) {
		mvwaddch(win, RRANDH, RRANDW, ROOM);
		wrefresh(win);
	}
}

size_t
wall_count(WINDOW *win, int x, int y)
{
	int i, j;
	size_t c = 0;

	for (i = x - 1; i <= x + 1; ++i) {
		for (j = y - 1; j <= y + 1; ++j) {
			if (mvwinch(win, j, i) == ROOM) c++;
		}
	}

	return c;
}

void
smooth_floor(WINDOW *win, size_t cutoff)
{
	int i, j;
	size_t wc;

	for (i = 1; i < WIDTH - 1; ++i) {
		for (j = 1; j < HEIGHT - 1; ++j) {
			wc = wall_count(win, i, j);

			if (wc < cutoff) {
				mvwaddch(win, j, i, ROCK);
			} else if (wc > cutoff) {
				mvwaddch(win, j, i, ROOM);
			}
		}
	}
}
*/

