#ifndef ROOM_H
#define ROOM_H

#include <ncurses.h>
#include <stdbool.h>

#define ROOM		'.'
#define CORRIDOR	'#'
#define ROCK		' '
#define STAIR_UP	'<'
#define STAIR_DOWN	'>'

struct room {
	int x;
	int y;
	int size_x;
	int size_y;
};

void	gen_room(struct room *const, int, int);
bool	draw_room(WINDOW *const, struct room const);
void	draw_corridor(WINDOW *const, struct room const, struct room const);

#endif /* ROOM_H */
