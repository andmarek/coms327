#ifndef OPAL_H
#define OPAL_H

#include "floor.h"

extern struct player p;

extern uint16_t room_count;
extern struct room *rooms;

extern uint16_t stair_up_count;
extern uint16_t stair_dn_count;

extern struct stair *stairs_up;
extern struct stair *stairs_dn;

extern int const width;
extern int const height;

struct tile {
	uint8_t	h; /* hardness */
	chtype	c; /* character */
};

extern struct tile *tiles;

#endif /* OPAL_H */
