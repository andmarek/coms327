#ifndef GEN_H
#define GEN_H

#include <string>

std::string	rlg_path();

/* io */
bool	save_dungeon();
bool	load_dungeon();

/* gen */
void	clear_tiles();
void	arrange_new();
void	arrange_loaded();
void	arrange_renew();

#endif /* GEN_H */
