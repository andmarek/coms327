#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <vector>

#include "cerr.h"
#include "dijk.h"
#include "globs.h"
#include "npc.h"

static bool	valid_npc(uint8_t const, uint8_t const);

static double		distance(uint8_t const, uint8_t const, uint8_t const, uint8_t const);
static unsigned int	subu32(unsigned int const, unsigned int const);

static bool	pc_visible(int const, int const);

static unsigned int constexpr	limited_int_to_char(uint8_t const);

static void	move_redraw(WINDOW *const, npc &, uint8_t const, uint8_t const);
static void	move_tunnel(WINDOW *const, npc &, uint8_t const, uint8_t const);

static void	move_straight(WINDOW *const, npc &);
static void	move_dijk_nontunneling(WINDOW *const, npc &);
static void	move_dijk_tunneling(WINDOW *const, npc &);

static npc	gen_monster();

enum pc_action {
	PC_DEFOG,
	PC_MONSTER_LIST,
	PC_NEXT,
	PC_NONE,
	PC_QUIT,
	PC_TELE
};

static enum pc_action	turn_npc(WINDOW *const, npc &);
static enum pc_action	turn_pc(WINDOW *const, npc &);

static void	monster_list(WINDOW *const, std::vector<npc> const &);

static bool	viewable(int const, int const);
static void	pc_viewbox(WINDOW *const, int const);

class compare_npc {
public:
	bool constexpr
	operator() (npc const &x, npc const &y) const
	{
		return x.turn > y.turn;
	}
};

static int constexpr PLAYER_TYPE = 0x80;
static int constexpr INTELLIGENT = 0x1;
static int constexpr TELEPATHIC = 0x2;
static int constexpr TUNNELING = 0x4;
static int constexpr ERRATIC = 0x8;

/* minimum distance from the PC an NPC can be placed */
static double constexpr CUTOFF = 4.0;

static int constexpr PERSISTANCE = 5;

static int constexpr TYPE_MIN = 0x0;
static int constexpr TYPE_MAX = 0xF;
static int constexpr SPEED_MIN = 5;
static int constexpr SPEED_MAX = 20;

static int constexpr KEY_ESC = 27;

static int constexpr DEFAULT_LUMINANCE = 5;

npc player;

enum turn_exit
turn_engine(WINDOW *const win, unsigned int const nummon)
{
	std::priority_queue<npc, std::vector<std::reference_wrapper<npc>>, compare_npc> heap;
	std::vector<npc> monsters;

	/* monster list window */
	WINDOW *mwin;
	int32_t turn;
	unsigned int alive = nummon;
	enum turn_exit ret = TURN_NONE;

	monsters.resize(nummon);

	player.speed = 10;
	player.type = PLAYER_TYPE;
	player.turn = 0;
	player.type_ch = PLAYER;
	tiles[player.y][player.x].n = &player;

	(void)mvwaddch(win, player.y, player.x, player.type_ch);

	heap.push(player);

	for (auto &npc : monsters) {
		npc = gen_monster();
		tiles[npc.y][npc.x].n = &npc;

		heap.push(npc);
	}

	dijkstra();

	if ((mwin = newwin(HEIGHT, WIDTH, 0, 0)) == NULL) {
		cerrx(1, "turn_engine newwin mwin");
	}

	if (keypad(mwin, true) == ERR) {
		cerrx(1, "keypad mwin");
	}

	while (!heap.empty()) {
		npc &n = heap.top();
		heap.pop();

		if (n.type & PLAYER_TYPE) {
			pc_viewbox(win, DEFAULT_LUMINANCE);

			if (wrefresh(win) == ERR) {
				cerrx(1, "turn_engine wrefresh");
			}
		}

		if (alive == 0) {
			ret = TURN_WIN;
			goto exit;
		}

		if (n.dead) {
			if (n.type & PLAYER_TYPE) {
				ret = TURN_DEATH;
				goto exit;
			}

			monsters.erase(std::remove_if(monsters.begin(),
				monsters.end(),
				[&n](npc const &o) { return &o == &n; }),
				monsters.end());
			alive--;
			continue;
		}

		turn = n.turn + 1;
		n.turn = turn + 1000/n.speed;

		retry:
		switch(turn_npc(win, n)) {
		case PC_DEFOG:
			/* TODO defog */
			goto retry;
		case PC_MONSTER_LIST:
			monster_list(mwin, monsters);
			touchwin(win);
			wrefresh(win);
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
			/* TODO teleport */
			goto retry;
		}

		heap.push(n);
	}

	exit:

	delwin(mwin);

	return ret;
}

static bool
valid_npc(uint8_t const y, uint8_t const x)
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

static unsigned int constexpr
limited_int_to_char(uint8_t const i)
{
	return (unsigned int)((i < 0xA) ? i + '0' : i + 'a' - 0xA);
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

	if (tiles[n.y][n.x].v) {
		(void)mvwaddch(win, n.y, n.x, tiles[n.y][n.x].c);
	}

	if (tiles[y][x].v) {
		(void)mvwaddch(win, y, x, n.type_ch);
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

			if (!(n.type & TUNNELING) && tiles[y][x].h != 0) {
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

	if (n.type & TUNNELING) {
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

static npc
gen_monster()
{
	uint8_t x, y;
	npc n;

	do {
		x = rr.rrand<uint8_t>(1, WIDTH - 2);
		y = rr.rrand<uint8_t>(1, HEIGHT - 2);
	} while (!valid_npc(y, x));

	n.x = x;
	n.y = y;

	n.type = rr.rrand<uint8_t>(TYPE_MIN, TYPE_MAX);
	n.speed = rr.rrand<uint8_t>(SPEED_MIN, SPEED_MAX);

	n.turn = 0;
	n.p_count = 0;
	n.dead = false;

	n.type_ch = limited_int_to_char(n.type);

	tiles[y][x].n = &n;

	return n;
}

static enum pc_action
turn_npc(WINDOW *const win, npc &n)
{
	if (n.type & PLAYER_TYPE) {
		return turn_pc(win, n);
	}

	if (n.type & ERRATIC && rr.rrand<int>(0, 1) == 0) {
		uint8_t y, x;

		do {
			y = (uint8_t)(n.y + rr.rrand<int>(-1, 1));
			x = (uint8_t)(n.x + rr.rrand<int>(-1, 1));
		} while (!(n.type & TUNNELING) && tiles[y][x].h != 0);

		if (n.type & TUNNELING) {
			move_tunnel(win, n, y, x);
		} else {
			move_redraw(win, n, y, x);
		}

		return PC_NONE;
	}

	switch(n.type) {
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
		if (n.type & TELEPATHIC || pc_visible(n.x, n.y)) {
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
		if (n.type & TELEPATHIC || pc_visible(n.x, n.y)) {
			n.p_count = PERSISTANCE;
		}

		if (n.p_count != 0) {
			move_dijk_tunneling(win, n);
			n.p_count--;
		}
		break;
	default:
		cerrx(1, "gen_monster invalid monster type %d", n.type);
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
			return PC_MONSTER_LIST;
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
monster_list(WINDOW *const mwin, std::vector<npc> const &monsters)
{
	std::vector<npc>::size_type cpos = 0;
	std::size_t i;

	while(1) {
		if (wclear(mwin) == ERR) {
			cerrx(1, "monster_list clear");
		}

		(void)box(mwin, 0, 0);

		(void)mvwprintw(mwin, HEIGHT - 1, 2,
			"[arrow keys to scroll; ESC to exit]");

		for (i = 0; i < HEIGHT - 2 && i + cpos < monsters.size(); ++i) {
			npc n = monsters[i + cpos];
			int dx = player.x - n.x;
			int dy = player.y - n.y;

			(void)mvwprintw(mwin, static_cast<int>(i + 1U), 2,
				"%u.\t%c, %d %s and %d %s", i + cpos, n.type_ch,
				abs(dy), dy > 0 ? "north" : "south",
				abs(dx), dx > 0 ? "east" : "west");
		}

		for (; i < HEIGHT - 2; ++i) {
			(void)mvwaddch(mwin, static_cast<int>(i + 1U), 2, '~');
		}

		wrefresh(mwin);

		switch(wgetch(mwin)) {
		case ERR:
			cerrx(1, "monster_list wgetch ERR");
			return;
		case KEY_UP:
			if (--cpos > monsters.size()) {
				cpos = 0;
			}
			break;
		case KEY_DOWN:
			if (++cpos > monsters.size() - 1) {
				cpos = monsters.size() - 1;
			}
			break;
		case 27:
			return;
		default:
			break;
		}
	}
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
			if (viewable(j, i)) {
				chtype ch = (tiles[j][i].n == NULL)
					? tiles[j][i].c
					: tiles[j][i].n->type_ch;
				tiles[j][i].v = true;
				(void)mvwaddch(win, j, i, ch);
			}
		}
	}
}
