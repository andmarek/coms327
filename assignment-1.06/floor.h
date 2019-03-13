#ifndef ROOM_H
#define ROOM_H

#include "globs.h"

void	gen_room(room *const);
bool	valid_room(room const *const);
void	draw_room(WINDOW *const, room const *const);
void	draw_corridor(WINDOW *const, room const, room const);
void	gen_draw_stair(WINDOW *const, stair *const, bool const);

#endif /* ROOM_H */
