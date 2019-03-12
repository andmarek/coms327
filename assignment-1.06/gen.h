#ifndef GEN_H
#define GEN_H

#include <ncurses.h>

void	clear_tiles(void);

void	arrange_new(WINDOW *const);
void	arrange_loaded(WINDOW *const);

void	arrange_renew(WINDOW *const);

#endif /* GEN_H */
