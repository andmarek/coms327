#ifndef GLOBS_H
#define GLOBS_H

#include <ncurses.h>
#include <stdint.h>

#define WIDTH	80
#define HEIGHT	21

#define PLAYER		'@'
#define ROOM		'.'
#define CORRIDOR	'#'
#define ROCK		' '
#define STAIR_UP	'<'
#define STAIR_DN	'>'

struct npc {
	uint8_t		x;
	uint8_t		y;
	uint8_t		speed;
	uint8_t		type;
	unsigned int	type_ch;
	int32_t		turn;
	uint8_t 	p_count;
	uint8_t		dead : 1;
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
	uint8_t	h; /* hardness */
	chtype	c; /* character */

	/* dijstra */
	int32_t	d; /* non-tunneling distance cost */
	int32_t dt; /* tunneling distance cost */
	uint8_t	x; /* x position */
	uint8_t	y; /* y position */

	/* turn engine */
	struct npc	*n;
};

extern struct tile tiles[HEIGHT][WIDTH];

extern struct npc player;

extern uint16_t room_count;
extern struct room *rooms;

extern uint16_t stair_up_count;
extern uint16_t stair_dn_count;

extern struct stair *stairs_up;
extern struct stair *stairs_dn;

#endif /* GLOBS_H */
