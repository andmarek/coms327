#ifndef GLOBS_H
#define GLOBS_H

#include <ncurses.h>
#include <cstdint>

#include "rand.h"

int constexpr WIDTH = 80;
int constexpr HEIGHT = 21;

char constexpr PLAYER = '@';
char constexpr ROOM = '.';
char constexpr CORRIDOR = '#';
char constexpr ROCK = ' ';
char constexpr STAIR_UP = '<';
char constexpr STAIR_DN = '>';

struct npc {
	uint8_t		x;
	uint8_t		y;
	uint8_t		speed;
	uint8_t		type;
	unsigned int	type_ch;
	int32_t		turn;
	uint8_t 	p_count;
	bool		dead;
};

struct room {
	uint8_t x;
	uint8_t y;
	uint8_t size_x;
	uint8_t size_y;
};

struct stair {
	uint8_t x;
	uint8_t y;
};

struct tile {
	/* turn engine */
	struct npc	*n;

	uint8_t	h; /* hardness */
	chtype	c; /* character */

	/* dijstra */
	int32_t	d; /* non-tunneling distance cost */
	int32_t dt; /* tunneling distance cost */
	uint8_t	x; /* x position */
	uint8_t	y; /* y position */
	bool	vd; /* valid non-tunneling node available for processing */
	bool	vdt; /* valid tunneling node available for processing */
};

extern ranged_random rr;

extern struct npc player;

extern struct tile tiles[HEIGHT][WIDTH];

extern uint16_t room_count;
extern std::vector<struct room> rooms;

extern uint16_t stair_up_count;
extern uint16_t stair_dn_count;

extern std::vector<struct stair> stairs_up;
extern std::vector<struct stair> stairs_dn;

#endif /* GLOBS_H */
