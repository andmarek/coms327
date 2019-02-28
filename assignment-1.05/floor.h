#ifndef ROOM_H
#define ROOM_H

#include "globs.h"

void	gen_room(struct room *const);
int	valid_room(struct room const *const);
void	draw_room(WINDOW *const, struct room const *const);
void	draw_corridor(WINDOW *const, struct room const, struct room const);
void	gen_draw_stair(WINDOW *const, struct stair *const, bool const);

#endif /* ROOM_H */
