#ifndef PLAYER_H
#define PLAYER_H

#include <ncurses.h>
#include <inttypes.h>

#define PLAYER	'@'

struct player {
	uint8_t x;
	uint8_t y;
};

void	place_player(WINDOW *const, struct player *const, int, int);

#endif /* PLAYER_H */
