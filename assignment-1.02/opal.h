#ifndef OPAL_H
#define OPAL_H

#include "floor.h"

extern struct player p;

extern uint8_t room_count;
extern struct room *rooms;

extern uint8_t stair_up_count;
extern uint8_t stair_dn_count;

extern struct stair *stairs_up;
extern struct stair *stairs_dn;

extern int const width;
extern int const height;

struct tile {
	uint8_t h; /* hardness */
};

extern struct tile *tiles;

#endif /* OPAL_H */
