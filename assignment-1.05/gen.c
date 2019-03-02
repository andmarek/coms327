#include <stdlib.h>

#include "cerr.h"
#include "floor.h"
#include "globs.h"
#include "rand.h"

#define NEW_ROOM_COUNT	8
#define ROOM_RETRIES	150

static void	init_fresh(WINDOW *const win);

static void	place_player(void);
static int	valid_player(int const, int const);

void
clear_tiles(void)
{
	uint8_t i, j;

	for (i = 0; i < HEIGHT; ++i) {
		for (j = 0; j < WIDTH; ++j) {
			tiles[i][j] = (struct tile) {
				.h = 0,
				.c = 0,
				.d = 0,
				.dt = 0,
				.x = j,
				.y = i,
				.n = NULL,
			};

			if (i == 0 || j == 0 || i == HEIGHT - 1
				|| j == WIDTH - 1) {
				tiles[i][j].h = UINT8_MAX;
			} else {
				tiles[i][j].c = ROCK;
			}
		}
	}
}

void
arrange_new(WINDOW *const win)
{
	room_count = NEW_ROOM_COUNT;
	stair_up_count = (uint16_t)rrand(1, (room_count / 4) + 1);
	stair_dn_count = (uint16_t)rrand(1, (room_count / 4) + 1);

	if (!(rooms = malloc(sizeof(struct room) * room_count))
		&& room_count != 0) {
		cerr(1, "arrange_new malloc rooms");
	}

	if (!(stairs_up = malloc(sizeof(struct stair) * stair_up_count))
		&& stair_up_count != 0) {
		cerr(1, "arrange_new malloc stairs_dn");
	}

	if (!(stairs_dn = malloc(sizeof(struct stair) * stair_dn_count))
		&& stair_dn_count != 0) {
		cerr(1, "arrange_new malloc stairs_dn");
	}

	init_fresh(win);
}

void
arrange_loaded(WINDOW *const win)
{
	uint8_t i, j;
	uint16_t k;

	for (k = 0; k < room_count; ++k) {
		draw_room(win, &rooms[k]);
	}

	for (k = 0; k < stair_up_count; ++k) {
		tiles[stairs_up[k].y][stairs_up[k].x].c = STAIR_UP;
		(void)mvwaddch(win, stairs_up[k].y, stairs_up[k].x, STAIR_UP);
	}

	for (k = 0; k < stair_dn_count; ++k) {
		tiles[stairs_dn[k].y][stairs_dn[k].x].c = STAIR_DN;
		(void)mvwaddch(win, stairs_dn[k].y, stairs_dn[k].x, STAIR_DN);
	}

	for (i = 1; i < HEIGHT - 1; ++i) {
		for (j = 1; j < WIDTH - 1; ++j) {
			if (tiles[i][j].h == 0 && tiles[i][j].c == ROCK) {
				tiles[i][j].c = CORRIDOR;
				(void)mvwaddch(win, i, j, CORRIDOR);
			}
		}
	}
}

void
arrange_renew(WINDOW *const win)
{
	if (wclear(win) == ERR) {
		cerrx(1, "arrange_renew clear");
	}

	(void)box(win, 0, 0);

	clear_tiles();

	room_count = NEW_ROOM_COUNT;
	stair_up_count = (uint16_t)rrand(1, (room_count / 4) + 1);
	stair_dn_count = (uint16_t)rrand(1, (room_count / 4) + 1);

	if (!(rooms = realloc(rooms, sizeof(struct room) * room_count))
		&& room_count != 0) {
		cerr(1, "arrange_renew realloc rooms");
	}

	if (!(stairs_up = realloc(stairs_up, sizeof(struct stair) * stair_up_count))
		&& stair_up_count != 0) {
		cerr(1, "arrange_renew realloc stairs_up");
	}

	if (!(stairs_dn = realloc(stairs_dn, sizeof(struct stair) * stair_dn_count))
		&& stair_dn_count != 0) {
		cerr(1, "arrange_renew realloc stairs_dn");
	}

	init_fresh(win);
}

static void
init_fresh(WINDOW *const win)
{
	size_t i, j;
	size_t retries = 0;

	for (i = 0; i < room_count; ++i) {
		gen_room(&rooms[i]);
	}

	for (i = 0; i < room_count && retries < ROOM_RETRIES; ++i) {
		if (valid_room(&rooms[i]) == -1) {
			retries++;
			gen_room(&rooms[i--]);
		} else {
			draw_room(win, &rooms[i]);
		}
	}

	if (i < room_count) {
		if (i == 0) {
			cerrx(1, "unable to place any rooms");
		}

		room_count = (uint16_t)i;

		rooms = realloc(rooms, sizeof(struct room) * room_count);

		if (!rooms) {
			cerr(1, "realloc rooms");
		}
	}

	for (i = 0; i < room_count - 1U; ++i) {
		draw_corridor(win, rooms[i], rooms[i+1]);
	}

	for (i = 0; i < stair_up_count; ++i) {
		gen_draw_stair(win, &stairs_up[i], true);
	}

	for (i = 0; i < stair_dn_count; ++i) {
		gen_draw_stair(win, &stairs_dn[i], false);
	}

	for (i = 1; i < HEIGHT - 1; ++i) {
		for (j = 1; j < WIDTH - 1; ++j) {
			switch(tiles[i][j].c) {
			case ROOM:
			case CORRIDOR:
			case STAIR_UP:
			case STAIR_DN:
				tiles[i][j].h = 0;
				break;
			default:
				tiles[i][j].h = (uint8_t)rrand(1, UINT8_MAX-1U);
			}
		}
	}

	place_player();

}

static void
place_player(void)
{
	int x, y;

	do {
		x = rrand(1, WIDTH - 2);
		y = rrand(1, HEIGHT - 2);
	} while (!valid_player(y, x));

	player.x = (uint8_t)x;
	player.y = (uint8_t)y;
}

static int
valid_player(int const y, int const x)
{
	return tiles[y][x].h == 0
		&& tiles[y + 1][x].h == 0 && tiles[y - 1][x].h == 0
		&& tiles[y][x + 1].h == 0 && tiles[y][x - 1].h == 0;
}
