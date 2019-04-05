#ifndef ROOM_H
#define ROOM_H

#include "globs.h"

bool	gen_room(room &);
void	draw_room(room const &);
void	gen_corridor(room const &, room const &);
void	gen_stair(stair &, bool const);

#endif /* ROOM_H */
