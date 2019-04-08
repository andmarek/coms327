#include <algorithm>
#include <functional>
#include <limits>
#include <new>
#include <queue>
#include <utility>
#include <vector>

#include "cerr.h"
#include "dijk.h"
#include "globs.h"
#include "turn.h"

static bool	valid_thing(uint8_t const, uint8_t const);

static double		distance(uint8_t const, uint8_t const, uint8_t const, uint8_t const);
static unsigned int	subu32(unsigned int const, unsigned int const);

static bool	pc_visible(int const, int const);

static void	move_redraw(WINDOW *const, npc &, uint8_t const, uint8_t const);
static void	move_tunnel(WINDOW *const, npc &, uint8_t const, uint8_t const);

static void	move_straight(WINDOW *const, npc &);
static void	move_dijk_nontunneling(WINDOW *const, npc &);
static void	move_dijk_tunneling(WINDOW *const, npc &);

static void	gen_npc(npc &);
static std::optional<std::pair<uint8_t, uint8_t>>	gen_obj();

static void	npc_list(WINDOW *const, std::vector<npc *> const &);

static void	defog(WINDOW *const, std::vector<npc *> const &, std::vector<obj *> const &);

static void	crosshair(WINDOW *const, uint8_t const, uint8_t const);
static bool	teleport(WINDOW *const);

static bool	viewable(int const, int const);
static void	pc_viewbox(WINDOW *const, int const);

enum pc_action {
	PC_DEFOG,
	PC_NPC_LIST,
	PC_NEXT,
	PC_NONE,
	PC_QUIT,
	PC_TELE
};

static enum pc_action	turn_npc(WINDOW *const, npc &);
static enum pc_action	turn_pc(WINDOW *const, npc &);

struct compare_npc {
	bool constexpr
	operator() (npc const &x, npc const &y) const
	{
		return x.turn > y.turn;
	}
};

/* minimum distance from the PC an NPC can be placed */
static double constexpr CUTOFF = 4.0;
static int constexpr PERSISTANCE = 5;
static int constexpr KEY_ESC = 27;
static int constexpr DEFAULT_LUMINANCE = 5;
static unsigned int constexpr RETRIES = 150;

npc player;

enum turn_exit
turn_engine(WINDOW *const win, unsigned int const numnpcs,
	unsigned int const numobjs)
{
	std::priority_queue<npc, std::vector<std::reference_wrapper<npc>>, compare_npc> heap;
	std::vector<npc *> npcs;
	std::vector<obj *> objs;
	unsigned int real_num = 0;

	/* npc list, defogged windows */
	WINDOW *nwin, *fwin;

	int32_t turn;
	int alive = numnpcs;
	enum turn_exit ret = TURN_NONE;

	try {
		npcs.resize(numnpcs);
		objs.resize(numobjs);
	} catch (std::bad_alloc const &) {
		cerr(1, "resize npcs and objs");
	}
	player.speed = 10;
	player.type = PLAYER_TYPE;
	player.turn = 0;
	player.symb = PLAYER;
	tiles[player.y][player.x].n = &player;
	player.color = COLOR_PAIR(COLOR_YELLOW);

	wattron(win, player.color);
	(void)mvwaddch(win, player.y, player.x, player.symb);
	wattroff(win, player.color);

	heap.push(player);

	for (auto &n : npcs) {
		size_t i;
		unsigned int retries = 0;
		do {
			i = rr.rrand<size_t>(0, npcs_parsed.size() - 1);
			retries++;
		} while (retries < RETRIES && (npcs_parsed[i].done
			|| npcs_parsed[i].rrty <= rr.rrand<uint8_t>(0, 99)));

		if (retries == RETRIES) {
			break;
		}

		real_num++;

		n = new npc(npcs_parsed[i]);

		if (n->type & UNIQ) {
			n->done = true;
			npcs_parsed[i].done = true;
		}

		gen_npc(*n);

		tiles[n->y][n->x].n = n;

		heap.push(*n);
	}

	if (real_num != numnpcs) {
		npcs.resize(real_num);
		alive = real_num;
	}

	real_num = 0;

	for (auto &o : objs) {
		size_t i = 0;
		unsigned int retries = 0;
		do {
			i = rr.rrand<size_t>(0, objs_parsed.size() - 1);
			retries++;
		} while (retries < RETRIES && (objs_parsed[i].done
			|| objs_parsed[i].rrty <= rr.rrand<uint8_t>(0, 99)));

		if (retries == RETRIES) {
			break;
		}

		std::optional<std::pair<uint8_t, uint8_t>> coords = gen_obj();

		if (!coords.has_value()) {
			break;
		}

		real_num++;

		o = new obj(objs_parsed[i]);

		if (o->art) {
			o->done = true;
			objs_parsed[i].done = true;
		}

		o->x = (*coords).first;
		o->y = (*coords).second;

		tiles[o->y][o->x].o = o;
	}

	if (real_num != numobjs) {
		objs.resize(real_num);
	}

	dijkstra();

	if ((nwin = newwin(HEIGHT, WIDTH, 0, 0)) == NULL
		|| (fwin = newwin(HEIGHT, WIDTH, 0, 0)) == NULL) {
		cerrx(1, "turn_engine newwin nwin, fwin");
	}

	if (keypad(nwin, true) == ERR) {
		cerrx(1, "keypad nwin");
	}

	(void)box(fwin, 0, 0);

	pc_viewbox(win, DEFAULT_LUMINANCE);

	while (!heap.empty()) {
		npc &n = heap.top();
		heap.pop();

		if (n.type & PLAYER_TYPE && wrefresh(win) == ERR) {
			cerrx(1, "turn_engine wrefresh");
		}

		npcs.erase(std::remove_if(npcs.begin(), npcs.end(),
			[&alive](auto const o)
			{
				if (o->dead) {
					alive--;
					o->~npc();
					return true;
				} else {
					return false;
				}
			}), npcs.end());

		if (alive <= 0) {
			if (n.type & PLAYER_TYPE) {
				ret = TURN_WIN;
				goto exit;
			}

			continue;
		}


		if (n.dead) {
			if (n.type & PLAYER_TYPE) {
				ret = TURN_DEATH;
				goto exit;
			} else if (n.type & BOSS) {
				ret = TURN_WIN;
				goto exit;
			}

			continue;
		}

		turn = n.turn + 1;
		n.turn = turn + 1000/n.speed;

		retry:
		switch(turn_npc(win, n)) {
		case PC_DEFOG:
			defog(fwin, npcs, objs);

			if (touchwin(win) == ERR) {
				cerrx(1, "touchwin fwin");
			}

			goto retry;
		case PC_NPC_LIST:
			npc_list(nwin, npcs);

			if (touchwin(win) == ERR) {
				cerrx(1, "touchwin nwin");
			}

			goto retry;
		case PC_NEXT:
			ret = TURN_NEXT;
			goto exit;
		case PC_NONE:
			break;
		case PC_QUIT:
			ret = TURN_QUIT;
			goto exit;
		case PC_TELE:
			bool act = teleport(win);

			if (touchwin(win) == ERR) {
				cerrx(1, "touchwin twin");
			}

			if (act) {
				break;
			} else {
				goto retry;
			}
		}

		heap.push(n);
	}

	exit:

	if (delwin(nwin) == ERR) {
		cerrx(1, "turn_engine delwin nwin");
	}

	if (delwin(fwin) == ERR) {
		cerrx(1, "turn_engine delwin fwin");
	}

	return ret;
}

static bool
valid_thing(uint8_t const y, uint8_t const x)
{
	if (tiles[y][x].h != 0) {
		return false;
	}

	return distance(player.x, player.y, x, y) > CUTOFF;
}

static double
distance(uint8_t const x0, uint8_t const y0, uint8_t const x1, uint8_t const y1)
{
	int const dx = x1 - x0;
	int const dy = y1 - y0;
	return std::sqrt(dx * dx + dy * dy);
}

/*
 * From my branchfree saturating arithmetic library.
 * See: https://github.com/esote/bsa
 */
static unsigned int
subu32(unsigned int const a, unsigned int const b)
{
	unsigned int res = a - b;
	res &= (unsigned int) (-(unsigned int) (res <= a));
	return res;
}

/* Bresenham's line algorithm */
static bool
pc_visible(int const x1, int const y1)
{
	int x0 = player.x;
	int y0 = player.y;

	int const dx = std::abs(x1 - x0);
	int const dy = std::abs(y1 - y0);
	int const sx = x0 < x1 ? 1 : -1;
	int const sy = y0 < y1 ? 1 : -1;

	int err = (dx > dy ? dx : -dy) / 2;

	while(1) {
		if (tiles[y0][x0].h != 0) {
			return false;
		}

		if (x0 == x1 && y0 == y1) {
			break;
		}

		int e2 = err;

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

static void
move_redraw(WINDOW *const win, npc &n, uint8_t const y, uint8_t const x)
{
	if (n.x == x && n.y == y) {
		return;
	}

	if (tiles[y][x].n != NULL) {
		(tiles[y][x].n)->dead = true;
	}

	tiles[n.y][n.x].n = NULL;
	tiles[y][x].n = &n;

	if (tiles[n.y][n.x].v || n.type & PLAYER_TYPE) {
		if (tiles[n.y][n.x].o != NULL) {
			wattron(win, tiles[n.y][n.x].o->color);
			(void)mvwaddch(win, n.y, n.x, tiles[n.y][n.x].o->symb);
			wattroff(win, tiles[n.y][n.x].o->color);
		} else {
			(void)mvwaddch(win, n.y, n.x, tiles[n.y][n.x].c);
		}
	}

	if (tiles[y][x].v) {
		wattron(win, n.color);
		(void)mvwaddch(win, y, x, n.symb);
		wattroff(win, n.color);
	}

	n.y = y;
	n.x = x;
}

static void
move_tunnel(WINDOW *const win, npc &n, uint8_t const y, uint8_t const x)
{
	if (tiles[y][x].h == UINT8_MAX) {
		return;
	}

	tiles[y][x].h = (uint8_t)subu32(tiles[y][x].h, TUNNEL_STRENGTH);

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
move_straight(WINDOW *const win, npc &n)
{
	double min = std::numeric_limits<double>::max();
	uint8_t minx = n.x;
	uint8_t miny = n.y;

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			uint8_t x = (uint8_t)(n.x + i);
			uint8_t y = (uint8_t)(n.y + j);

			if (!(n.type & TUNNEL) && tiles[y][x].h != 0) {
				continue;
			}

			double dist = distance(player.x, player.y, x, y);

			if (dist < min) {
				min = dist;
				minx = x;
				miny = y;
			}
		}
	}

	if (n.type & TUNNEL) {
		move_tunnel(win, n, miny, minx);
	} else {
		move_redraw(win, n, miny, minx);
	}
}

static void
move_dijk_nontunneling(WINDOW *const win, npc &n)
{
	int32_t min_d = tiles[n.y][n.x].d;
	uint8_t minx = n.x;
	uint8_t miny = n.y;

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			uint8_t x = (uint8_t)(n.x + i);
			uint8_t y = (uint8_t)(n.y + j);


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
move_dijk_tunneling(WINDOW *const win, npc &n)
{
	int32_t min_dt = tiles[n.y][n.x].dt;
	uint8_t minx = n.x;
	uint8_t miny = n.y;

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			uint8_t x = (uint8_t)(n.x + i);
			uint8_t y = (uint8_t)(n.y + j);

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
gen_npc(npc &n)
{
	uint8_t x, y;

	do {
		x = rr.rrand<uint8_t>(1, WIDTH - 2);
		y = rr.rrand<uint8_t>(1, HEIGHT - 2);
	} while (!valid_thing(y, x));

	n.x = x;
	n.y = y;
}

static std::optional<std::pair<uint8_t, uint8_t>>
gen_obj()
{
	uint8_t x, y;
	size_t retries = 0;

	do {
		x = rr.rrand<uint8_t>(1, WIDTH - 2);
		y = rr.rrand<uint8_t>(1, HEIGHT - 2);
		retries++;
	} while (retries < RETRIES && (!valid_thing(y, x)
		|| tiles[y][x].o != NULL));

	if (retries == RETRIES) {
		return {};
	}

	return std::make_pair(x, y);
}

static enum pc_action
turn_npc(WINDOW *const win, npc &n)
{
	if (n.type & PLAYER_TYPE) {
		pc_viewbox(win, DEFAULT_LUMINANCE);
		return turn_pc(win, n);
	}

	if (n.type & ERRATIC && rr.rrand<int>(0, 1) == 0) {
		uint8_t y, x;

		do {
			y = (uint8_t)(n.y + rr.rrand<int>(-1, 1));
			x = (uint8_t)(n.x + rr.rrand<int>(-1, 1));
		} while (!(n.type & TUNNEL) && tiles[y][x].h != 0);

		if (n.type & TUNNEL) {
			move_tunnel(win, n, y, x);
		} else {
			move_redraw(win, n, y, x);
		}

		return PC_NONE;
	}

	uint16_t const basic_type = n.type & 0xF;

	switch(basic_type) {
	case 0x0:
	case 0x8:
	case 0x4:
	case 0xC:
		/*
		 * straight line if can see player
		 * or tunnel: straight line if can see player
		 */
		if (pc_visible(n.x, n.y)) {
			move_straight(win, n);
		}
		break;
	case 0x2:
	case 0xA:
	case 0x6:
	case 0xE:
		/*
		 * straight line, telepathic towards player
		 * or tunnel: straight line, telepathic towards player
		 */
		move_straight(win, n);
		break;
	case 0x1:
	case 0x9:
	case 0x3:
	case 0xB:
		/*
		 * use nontunneling dijk, remembered location
		 * or use nontunneling dijk, telepathic towards player
		 */
		if (n.type & TELE || pc_visible(n.x, n.y)) {
			n.p_count = PERSISTANCE;
		}

		if (n.p_count != 0) {
			move_dijk_nontunneling(win, n);
			n.p_count--;
		}
		break;
	case 0x5:
	case 0xD:
	case 0x7:
	case 0xF:
		/*
		 * tunnel: use tunneling dijk, remembered location
		 * or tunnel: use tunneling dijk, telepathic towards player
		 */
		if (n.type & TELE || pc_visible(n.x, n.y)) {
			n.p_count = PERSISTANCE;
		}

		if (n.p_count != 0) {
			move_dijk_tunneling(win, n);
			n.p_count--;
		}
		break;
	default:
		cerrx(1, "turn_npc invalid npc type %d", n.type);
	}

	return PC_NONE;
}

static enum pc_action
turn_pc(WINDOW *const win, npc &n)
{
	uint8_t y = n.y;
	uint8_t x = n.x;
	bool exit = false;

	while(!exit) {
		exit = true;
		switch(wgetch(win)) {
		case ERR:
			cerrx(1, "turn_pc wgetch ERR");
			break;
		case KEY_HOME:
		case KEY_A1:
		case '7':
		case 'y':
			/* up left */
			y--;
			x--;
			break;
		case KEY_UP:
		case '8':
		case 'k':
			/* up */
			y--;
			break;
		case KEY_PPAGE:
		case KEY_A3:
		case '9':
		case 'u':
			/* up right */
			y--;
			x++;
			break;
		case KEY_RIGHT:
		case '6':
		case 'l':
			/* right */
			x++;
			break;
		case KEY_NPAGE:
		case KEY_C3:
		case '3':
		case 'n':
			/* down right */
			y++;
			x++;
			break;
		case KEY_DOWN:
		case '2':
		case 'j':
			/* down */
			y++;
			break;
		case KEY_END:
		case KEY_C1:
		case '1':
		case 'b':
			/* down left */
			y++;
			x--;
			break;
		case KEY_LEFT:
		case '4':
		case 'h':
			/* left */
			x--;
			break;
		case KEY_B2:
		case ' ':
		case '5':
		case '.':
			/* rest */
			return PC_NONE;
		case '>':
			/* go down stairs */
			if (tiles[y][x].c == STAIR_DN) {
				return PC_NEXT;
			} else {
				exit = false;
			}
			break;
		case '<':
			/* go up stairs */
			if (tiles[y][x].c == STAIR_UP) {
				return PC_NEXT;
			} else {
				exit = false;
			}
			break;
		case 'm':
			return PC_NPC_LIST;
		case 'Q':
		case 'q':
			/* quit game */
			return PC_QUIT;
		case 'f':
			/* defog */
			return PC_DEFOG;
		case 't':
			/* teleport */
			return PC_TELE;
		default:
			exit = false;
		}
	}

	if (tiles[y][x].h == 0 && (x != n.x || y != n.y)) {
		move_redraw(win, n, y, x);
		dijkstra();
	}

	return PC_NONE;
}

static void
npc_list(WINDOW *const nwin, std::vector<npc *> const &npcs)
{
	std::vector<npc>::size_type cpos = 0;

	while(1) {
		if (wclear(nwin) == ERR) {
			cerrx(1, "npc_list clear");
		}

		(void)box(nwin, 0, 0);

		(void)mvwprintw(nwin, HEIGHT - 1, 2,
			"[ arrow keys to scroll; ESC to exit ]");

		std::size_t i;
		for (i = 0; i < HEIGHT - 2 && i + cpos < npcs.size(); ++i) {
			npc *n = npcs[i + cpos];
			int dx = player.x - n->x;
			int dy = player.y - n->y;

			(void)mvwprintw(nwin, static_cast<int>(i + 1U), 2,
				"%u.\t%c, %d %s and %d %s", i + cpos, n->symb,
				abs(dy), dy > 0 ? "north" : "south",
				abs(dx), dx > 0 ? "west" : "east");
		}

		for (; i < HEIGHT - 2; ++i) {
			(void)mvwaddch(nwin, static_cast<int>(i + 1U), 2, '~');
		}

		if (wrefresh(nwin) == ERR) {
			cerrx(1, "npc_list wrefresh");
		}

		switch(wgetch(nwin)) {
		case ERR:
			cerrx(1, "npc_list wgetch ERR");
			return;
		case KEY_UP:
			if (--cpos > npcs.size()) {
				cpos = 0;
			}
			break;
		case KEY_DOWN:
			if (++cpos > npcs.size() - 1) {
				cpos = npcs.size() - 1;
			}
			break;
		case KEY_ESC:
			return;
		default:
			break;
		}
	}
}

static void
defog(WINDOW *const fwin, std::vector<npc *> const &npcs,
	std::vector<obj *> const &objs)
{
	for (int x = 1; x < WIDTH - 1; ++x) {
		for (int y = 1; y < HEIGHT - 1; ++y) {
			(void)mvwaddch(fwin, y, x, tiles[y][x].c);
		}
	}

	for (auto const &o : objs) {
		wattron(fwin, o->color);
		(void)mvwaddch(fwin, o->y, o->x, o->symb);
		wattroff(fwin, o->color);
	}

	for (auto const &n : npcs) {
		wattron(fwin, n->color);
		(void)mvwaddch(fwin, n->y, n->x, n->symb);
		wattroff(fwin, n->color);
	}

	wattron(fwin, player.color);
	(void)mvwaddch(fwin, player.y, player.x, player.symb);
	wattroff(fwin, player.color);

	(void)mvwprintw(fwin, HEIGHT - 1, 2, "[ press any key to exit ]");

	if (wrefresh(fwin) == ERR) {
		cerrx(1, "defog wrefresh");
	}

	if (wgetch(fwin) == ERR) {
		cerrx(1, "defog wgetch ERR");
	}
}

static void
crosshair(WINDOW *const twin, uint8_t const y, uint8_t const x)
{
	for (int i = 1; i < HEIGHT - 1; ++i) {
		if (i != y) {
			(void)mvwaddch(twin, i, x, ACS_VLINE);
		}
	}

	for (int i = 1; i < WIDTH - 1; ++i) {
		if (i != x) {
			(void)mvwaddch(twin, y, i, ACS_HLINE);
		}
	}

	(void)mvwaddch(twin, y, 0, ACS_LTEE);
	(void)mvwaddch(twin, y, WIDTH - 1, ACS_RTEE);
	(void)mvwaddch(twin, 0, x, ACS_TTEE);
	(void)mvwaddch(twin, HEIGHT - 1, x, ACS_BTEE);

	(void)mvwaddch(twin, y + 1, x + 0, ACS_TTEE);
	(void)mvwaddch(twin, y - 1, x + 0, ACS_BTEE);
	(void)mvwaddch(twin, y + 0, x - 1, ACS_RTEE);
	(void)mvwaddch(twin, y + 0, x + 1, ACS_LTEE);

	(void)mvwaddch(twin, y - 1, x - 1, ACS_ULCORNER);
	(void)mvwaddch(twin, y - 1, x + 1, ACS_URCORNER);
	(void)mvwaddch(twin, y + 1, x - 1, ACS_LLCORNER);
	(void)mvwaddch(twin, y + 1, x + 1, ACS_LRCORNER);
}

static bool
teleport(WINDOW *const win)
{
	WINDOW *twin;
	uint8_t y = player.y;
	uint8_t x = player.x;
	bool ret = true;

	while(1) {
		if ((twin = dupwin(win)) == NULL) {
			cerrx(1, "crosshair dupwin");
		}

		if (touchwin(twin) == ERR) {
			cerrx(1, "teleport touchwin");
		}

		crosshair(twin, y, x);

		(void)mvwprintw(twin, HEIGHT - 1, 2,
			"[ PC control keys; 'r' for random location; "
			"'t' to teleport; ESC to exit ]");


		if (wrefresh(twin) == ERR) {
			cerrx(1, "teleport wrefresh");
		}

		switch(wgetch(win)) {
		case ERR:
			cerrx(1, "teleport wgetch ERR");
			break;
		case KEY_HOME:
		case KEY_A1:
		case '7':
		case 'y':
			/* up left */
			y--;
			x--;
			break;
		case KEY_UP:
		case '8':
		case 'k':
			/* up */
			y--;
			break;
		case KEY_PPAGE:
		case KEY_A3:
		case '9':
		case 'u':
			/* up right */
			y--;
			x++;
			break;
		case KEY_RIGHT:
		case '6':
		case 'l':
			/* right */
			x++;
			break;
		case KEY_NPAGE:
		case KEY_C3:
		case '3':
		case 'n':
			/* down right */
			y++;
			x++;
			break;
		case KEY_DOWN:
		case '2':
		case 'j':
			/* down */
			y++;
			break;
		case KEY_END:
		case KEY_C1:
		case '1':
		case 'b':
			/* down left */
			y++;
			x--;
			break;
		case KEY_LEFT:
		case '4':
		case 'h':
			/* left */
			x--;
			break;
		case 'r':
			/* random teleport location */
			x = rr.rrand<uint8_t>(2, WIDTH - 1);
			y = rr.rrand<uint8_t>(2, HEIGHT - 1);
			break;
		case 't':
			/* complete teleport */
			tiles[y][x].v = true;
			move_redraw(win, player, y, x);
			goto exit;
		case KEY_ESC:
			ret = false;
			goto exit;
		default:
			break;
		}

		if (x >= WIDTH - 1) {
			x = WIDTH - 2;
		} else if (x < 1) {
			x = 1;
		}

		if (y >= HEIGHT - 1) {
			y = HEIGHT - 2;
		} else if (y < 1) {
			y = 1;
		}
	}

	exit:

	if (delwin(twin) == ERR) {
		cerrx(1, "teleport delwin");
	}

	return ret;
}

static bool
viewable(int const y, int const x)
{
	return !tiles[y][x].v && pc_visible(x, y)
		&& (pc_visible(x - 1, y + 0)
		|| pc_visible(x + 1, y + 0)
		|| pc_visible(x + 0, y - 1)
		|| pc_visible(x + 0, y + 1)
		|| pc_visible(x + 1, y + 1)
		|| pc_visible(x - 1, y - 1)
		|| pc_visible(x - 1, y + 1)
		|| pc_visible(x + 1, y - 1));
}

static void
pc_viewbox(WINDOW *const win, int const lum)
{
	int const start_x = subu32(player.x, lum) + 1;
	int const end_x = player.x + lum;

	int const start_y = subu32(player.y, lum) + 1;
	int const end_y = player.y + lum;

	for (int i = start_x; i <= end_x && i < WIDTH - 1; ++i) {
		for (int j = start_y; j <= end_y && j < HEIGHT - 1; ++j) {
			if (!viewable(j, i)) {
				continue;
			}

			if (tiles[j][i].n != NULL) {
				wattron(win, tiles[j][i].n->color);
				(void)mvwaddch(win, j, i, tiles[j][i].n->symb);
				wattroff(win, tiles[j][i].n->color);
			} else if (tiles[j][i].o != NULL) {
				wattron(win, tiles[j][i].o->color);
				(void)mvwaddch(win, j, i, tiles[j][i].o->symb);
				wattroff(win, tiles[j][i].o->color);
			} else {
				(void)mvwaddch(win, j, i, tiles[j][i].c);
			}
		}
	}
}
