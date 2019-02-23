#ifndef PLAYER_H
#define PLAYER_H

#include <ncurses.h>
#include <inttypes.h>

#include "opal.h"

#define PLAYER	'@'

struct player {
	uint8_t x;
	uint8_t y;
};

void	place_player(WINDOW *const, int const, int const);
int	move_player(WINDOW *const, uint8_t const, uint8_t const);

#endif /* PLAYER_H */
