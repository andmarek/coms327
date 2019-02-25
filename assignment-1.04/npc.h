#ifndef NPC_H
#define NPC_H

#include <ncurses.h>
#include <stdint.h>

struct npc {
	uint8_t	x;
	uint8_t	y;
	uint8_t	speed;
	uint8_t	type;
	int32_t	turn;
	uint8_t target_x;
	uint8_t target_y;
	bool	saw_pc;
	bool	dead;
};

extern struct npc player;
#define PLAYER	'@'

void	place_player(WINDOW *const, int const, int const);

void	turn_engine(WINDOW *const, unsigned int const, int const, int const);

#endif /* NPC_H */
