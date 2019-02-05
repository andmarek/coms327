#ifndef ROOM_H
#define ROOM_H

#include <ncurses.h>

#define ROOM		'.'
#define CORRIDOR	'#'
#define ROCK		' '
#define STAIR_UP	'<'
#define STAIR_DN	'>'

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

void	gen_room(struct room *const, int, int);
int	draw_room(WINDOW *const, struct room const *const);
void	draw_corridor(WINDOW *const, struct room const, struct room const);
void	gen_draw_stair(WINDOW *const, struct stair *const, int, int, int);

#endif /* ROOM_H */
