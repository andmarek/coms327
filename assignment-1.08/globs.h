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

enum color {
	black,
	blue,
	cyan,
	green,
	magenta,
	red,
	white,
	yellow
};

uint16_t constexpr SMART = 1 << 0;
uint16_t constexpr TELE = 1 << 1;
uint16_t constexpr TUNNEL = 1 << 2;
uint16_t constexpr ERRATIC = 1 << 3;

uint16_t constexpr PLAYER_TYPE = 1 << 9;

uint16_t constexpr BOSS = 1 << 4;
uint16_t constexpr DESTROY = 1 << 5;
uint16_t constexpr PASS = 1 << 6;
uint16_t constexpr PICKUP = 1 << 7;
uint16_t constexpr UNIQ = 1 << 8;

struct dice {
	uint8_t base;
	uint8_t	dice;
	uint8_t	sides;
};

struct npc {
	uint8_t		x;
	uint8_t		y;
	uint8_t		speed;
	uint16_t	type;
	int32_t		turn;
	uint8_t 	p_count;
	bool		dead;

	std::vector<color>	colors;
	dice			dam;
	std::string		desc;
	uint8_t			hp;
	std::string		name;
	int			rrty;
	unsigned int		symb;
};

enum type {
	ammunition,
	amulet,
	armor,
	book,
	boots,
	cloak,
	container,
	flask,
	food,
	gloves,
	gold,
	helmet,
	light,
	offhand,
	ranged,
	ring,
	scroll_type, /* avoid conflict with ncurses */
	wand,
	weapon,
};

struct obj {
	uint8_t 		attr;
	std::vector<color>	colors;
	dice			dam;
	uint8_t			def;
	std::string		desc;
	uint8_t			dodge;
	uint8_t			hit;
	std::string		name;
	uint8_t			rrty;
	uint8_t			speed;
	type			obj_type;
	uint8_t			val;
	uint8_t			weight;

	bool			art;
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
	npc	*n;
	obj	*o;

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

extern std::vector<npc> npcs_parsed;
extern std::vector<obj> objs_parsed;

#endif /* GLOBS_H */
