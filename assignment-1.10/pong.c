#if __STDC_VERSION__ >= 199901L || __ISOC99_SOURCE || _BSD_SOURCE \
	|| _SVID_SOURCE
#define C99
#endif

#include <err.h>
#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#define B_S	'o'
#define P_S	ACS_CKBOARD

struct ball {
	size_t x;
	size_t y;
	int dx;
	int dy;
};

struct paddle {
	size_t x;
	size_t y;
};

static int	collision(size_t const, struct paddle const *const,
			size_t const, size_t const);
static void	update_b(size_t const, size_t const, size_t const,
			struct ball *const, struct paddle const *const,
			struct paddle const *const);
static void	update_r(size_t const, size_t const, struct ball const *const,
			struct paddle *const);
static int	update_l(WINDOW *const, size_t const, size_t const,
			struct paddle *const);
static int	render(WINDOW *const, size_t const, size_t const, size_t const,
			struct ball const *const, struct paddle const *const,
			struct paddle const *const);

static size_t score_l;
static size_t score_r;

int
main(int argc, char *argv[])
{
	WINDOW *win;
	char *msg;
	struct ball b;
	struct paddle l;
	struct paddle r;
	size_t p_h;
	size_t height;
	size_t width;
	int delay;

	delay = 50;

#ifdef C99
	if (argc == 2) {
		delay = atoi(argv[1]);
	} else if (argc > 2) {
		errx(1, "usage: pong [delay]");
	}
#else
	(void)argv;
	if (argc > 1) {
		errx(1, "usage: pong");
	}
#endif

	msg = NULL;
	srand(time(NULL));

	win = initscr();

	if (win == NULL) {
		errx(1, "initwin");
	}

	if (refresh() == ERR) {
		errx(1, "first refresh");
	}

	if (curs_set(0) == ERR) {
		errx(1, "curs_set");
	}

	if (noecho() == ERR) {
		errx(1, "noecho");
	}

	if (raw() == ERR) {
		errx(1, "raw");
	}

	wtimeout(win, delay);
	getmaxyx(win, height, width);

	p_h = height >> 2;

	b.x = width >> 1;
	b.y = height >> 1;
	b.dx = 1;
	b.dy = -1;

	l.x = 4;
	l.y = height >> 1;

	r.x = width - 5;
	r.y = height >> 1;

	while(getmaxyx(win, height, width), update_l(win, height, p_h, &l)) {
		if (width < 32 || height < 8) {
			msg = "dimensions too small";
			break;
		}

		if (b.y == 0 || b.y > height - 1 || b.x == 0
			|| b.x > width - 1) {
			b.x = width >> 1;
			b.y = height >> 1;
		}

		l.x = 4;
		r.x = width - 5;
		p_h = height >> 2;

		if (l.y + p_h >= height) {
			l.y = height - p_h - 1;
		}

		if (r.y + p_h >= height) {
			r.y = height - p_h - 1;
		}

		update_b(height, width, p_h, &b, &l, &r);
		update_r(height, p_h, &b, &r);

		if (render(win, height, width, p_h, &b, &l, &r)) {
			msg = "game over max score reached";
			break;
		}
	}

	if (delwin(win) == ERR) {
		errx(1, "delwin");
	}

	if (endwin() == ERR) {
		errx(1, "endwin");
	}

	if (msg != NULL) {
		errx(1, msg);
	}

	return 0;
}

static int
collision(size_t const p_h, struct paddle const *const p, size_t const f_x,
	size_t const f_y)
{
	return f_x == p->x && f_y >= p->y && f_y < p->y + p_h;
}

static void
update_b(size_t const h, size_t const w, size_t const p_h,
	struct ball *const b, struct paddle const *const l,
	struct paddle const *const r)
{
	size_t const f_x = b->x + b->dx;
	size_t const f_y = b->y + b->dy;

	if (collision(p_h, l, f_x, f_y) || collision(p_h, r, f_x, f_y)) {
		b->dx = -b->dx;
	} else if (f_x == 0) {
		score_r++;
		b->x = w - 4;
		b->y = rand() % (p_h - 1) + r->y;
		b->dx = -1;
	} else if (f_x == w - 1) {
		score_l++;
		b->x = 5;
		b->y = rand() % (p_h - 1) + l->y;
		b->dx = 1;
	}

	if (f_y == 0) {
		b->dy = 1;
	} else if (f_y == h - 1) {
		b->dy = -1;
	}

	b->x += b->dx;
	b->y += b->dy;
}

static void
update_r(size_t const h, size_t const p_h, struct ball const *const b,
	struct paddle *const r)
{
	size_t f_by;
	size_t f_py;
	size_t d;

	f_by = b->y + b->y * (r->x - b->x);

	if (f_by >= h) {
		return;
	}

	d = r->y + (p_h >> 1);

	f_py = r->y;

	if (b->y > d) {
		f_py++;
	} else if (b->y < d) {
		f_py--;
	}

	if (f_py + p_h < h && f_py > 0) {
		r->y = f_py;
	}
}

static int
update_l(WINDOW *const win, size_t const h, size_t const p_h,
	struct paddle *const l)
{
	switch(wgetch(win)) {
		case 'w':
			if (l->y - 1 != 0) {
				l->y--;
			}
			break;
		case 's':
			if (l->y + p_h < h - 1) {
				l->y++;
			}
			break;
		case 'q':
			return 0;
	}

	return 1;
}

static int
render(WINDOW *const win, size_t const h, size_t const w, size_t const p_h,
	struct ball const *const b, struct paddle const *const l,
	struct paddle const *const r)
{
	size_t i;

#ifndef SIZE_MAX
#error SIZE_MAX required to avoid overflow
#endif

	if (score_l == SIZE_MAX || score_r == SIZE_MAX) {
		return 1;
	}

	if (werase(win)) {
		errx(1, "werase");
	}

	(void)wborder(win, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD, ACS_CKBOARD);

	(void)mvwaddch(win, b->y, b->x, B_S);

	for (i = 1; i < h - 1; i += 2) {
		(void)mvwaddch(win, i, w >> 1, ACS_CKBOARD);
	}

	for (i = 0; i < p_h; ++i) {
		(void)mvwaddch(win, l->y + i, l->x, P_S);
		(void)mvwaddch(win, r->y + i, r->x, P_S);
	}

#ifdef C99
	(void)mvwprintw(win, 0, 2, "[left: %zd; right: %zd]", score_l, score_r);
#else
	(void)mvwprintw(win, 0, 2, "[left: %lu; right: %lu]", score_l, score_r);
#endif

	if (score_l == 47988) {
		(void)mvwprintw(win, 0, 2, "[416c6c20476f6f64205468696e6773]");
	}

	(void)mvwprintw(win, h - 1, 2, "[quit q; up/down w/s]");

	if (wrefresh(win) == ERR) {
		errx(1, "wrefresh");
	}

	return 0;
}
