#ifndef GLOBS_H
#define GLOBS_H

#include <ncurses.h>
#include <cstdint>
#include <vector>

#include "rand.h"

int constexpr WIDTH = 80;
int constexpr HEIGHT = 21;

int constexpr TUNNEL_STRENGTH = 85;

char constexpr PLAYER = '@';
char constexpr ROOM = '.';
char constexpr CORRIDOR = '#';
char constexpr ROCK = ' ';
char constexpr STAIR_UP = '<';
char constexpr STAIR_DN = '>';

class npc {
public:
	uint8_t		x;
	uint8_t		y;
	uint8_t		speed;
	uint8_t		type;
	unsigned int	type_ch;
	int32_t		turn;
	uint8_t 	p_count;
	bool		dead;
};

class room {
public:
	uint8_t x;
	uint8_t y;
	uint8_t size_x;
	uint8_t size_y;
};

class stair {
public:
	uint8_t x;
	uint8_t y;
};

class tile {
public:
	/* turn engine */
	npc	*n;

	uint8_t	h; /* hardness */
	chtype	c; /* character */

	uint8_t	x;
	uint8_t	y;

	/* dijkstra distance cost */
	int32_t	d;
	int32_t	dt;

	/* dijkstra valid node */
	bool	vd;
	bool	vdt;

	/* visited by PC */
	bool	v;
};

extern ranged_random rr;

extern npc player;

extern tile tiles[HEIGHT][WIDTH];

extern uint16_t room_count;
extern std::vector<room> rooms;

extern uint16_t stair_up_count;
extern uint16_t stair_dn_count;

extern std::vector<stair> stairs_up;
extern std::vector<stair> stairs_dn;

#endif /* GLOBS_H */
