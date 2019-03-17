#include "cerr.h"
#include "floor.h"
#include "globs.h"

static int constexpr NEW_ROOM_COUNT = 8;
static int constexpr ROOM_RETRIES = 150;

static void	init_fresh();

static void	place_player();
static int	valid_player(int const, int const);

void
clear_tiles()
{
	for (uint8_t i = 0; i < HEIGHT; ++i) {
		for (uint8_t j = 0; j < WIDTH; ++j) {
			tiles[i][j] = {};
			tiles[i][j].x = j;
			tiles[i][j].y = i;

			if (i == 0 || j == 0 || i == HEIGHT - 1
				|| j == WIDTH - 1) {
				tiles[i][j].h = std::numeric_limits<uint8_t>::max();
				tiles[i][j].d = std::numeric_limits<int32_t>::max();
				tiles[i][j].dt = std::numeric_limits<int32_t>::max();
			} else {
				tiles[i][j].c = ROCK;
			}
		}
	}
}

void
arrange_new()
{
	room_count = NEW_ROOM_COUNT;
	stair_up_count = rr.rrand<uint16_t>(1, (uint16_t)((room_count / 4) + 1));
	stair_dn_count = rr.rrand<uint16_t>(1, (uint16_t)((room_count / 4) + 1));

	rooms.resize(room_count);
	stairs_up.resize(stair_up_count);
	stairs_dn.resize(stair_dn_count);

	init_fresh();
}

void
arrange_loaded()
{
	for (auto const &r : rooms) {
		draw_room(r);
	}

	for (auto const &s : stairs_up) {
		tiles[s.y][s.x].c = STAIR_UP;
	}

	for (auto const &s : stairs_dn) {
		tiles[s.y][s.x].c = STAIR_DN;
	}

	for (int i = 1; i < HEIGHT - 1; ++i) {
		for (int j = 1; j < WIDTH - 1; ++j) {
			if (tiles[i][j].h == 0 && tiles[i][j].c == ROCK) {
				tiles[i][j].c = CORRIDOR;
			}
		}
	}
}

void
arrange_renew()
{
	clear_tiles();

	rooms.clear();
	stairs_up.clear();
	stairs_dn.clear();

	arrange_new();
}

static void
init_fresh()
{
	std::size_t i = 0;
	std::size_t retries = 0;

	for (auto it = rooms.begin(); it != rooms.end()
		&& retries < ROOM_RETRIES; ++it) {
		if (!gen_room(*it)) {
			retries++;
			it--;
		} else {
			i++;
			draw_room(*it);
		}
	}

	if (i < room_count) {
		if (i == 0) {
			cerrx(1, "unable to place any rooms");
		}

		room_count = (uint16_t)i;
		rooms.resize(room_count);
	}

	for (i = 0; i < room_count - 1U; ++i) {
		gen_corridor(rooms[i], rooms[i+1]);
	}

	for (auto &s : stairs_up) {
		gen_stair(s, true);
	}

	for (auto &s : stairs_dn) {
		gen_stair(s, true);
	}

	for (i = 1; i < HEIGHT - 1; ++i) {
		for (std::size_t j = 1; j < WIDTH - 1; ++j) {
			switch(tiles[i][j].c) {
			case ROOM:
			case CORRIDOR:
			case STAIR_UP:
			case STAIR_DN:
				tiles[i][j].h = 0;
				break;
			default:
				tiles[i][j].h = rr.rrand<uint8_t>(1,
					std::numeric_limits<uint8_t>::max() - 1);
			}
		}
	}

	place_player();
}

static void
place_player()
{
	uint8_t x, y;

	do {
		x = rr.rrand<uint8_t>(1, WIDTH - 2);
		y = rr.rrand<uint8_t>(1, HEIGHT - 2);
	} while (!valid_player(y, x));

	player.x = x;
	player.y = y;
}

static int
valid_player(int const y, int const x)
{
	return tiles[y][x].h == 0
		&& tiles[y + 1][x].h == 0 && tiles[y - 1][x].h == 0
		&& tiles[y][x + 1].h == 0 && tiles[y][x - 1].h == 0;
}
