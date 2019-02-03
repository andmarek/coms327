#ifndef OPAL_H
#define OPAL_H

#include "floor.h"

#define ROOM_COUNT	8

extern struct player p;

extern struct room rooms[ROOM_COUNT];

extern size_t stair_up_count;
extern size_t stair_dn_count;

extern struct stair *stairs_up;
extern struct stair *stairs_dn;

#endif /* OPAL_H */
