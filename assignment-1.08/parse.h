#ifndef PARSE_H
#define PARSE_H

#include <string>
#include <vector>

enum color {
	black,
	blue,
	cyan,
	green,
	magenta,
	red,
	white,
	yellow
};

enum ability {
	boss,
	destroy,
	erratic,
	pass,
	pickup,
	smart,
	tele,
	tunnel,
	uniq
};

struct dice {
	int	base;
	int	dice;
	int	sides;
};

struct npc_description {
	std::vector<ability>	abils;
	std::vector<color>	colors;
	dice			dam;
	std::string		desc;
	dice			hp;
	std::string		name;
	int			rrty;
	dice			speed;
	char			symb;
};

enum type {
	ammunition,
	amulet,
	armor,
	book,
	boots,
	cloak,
	container,
	flask,
	food,
	gloves,
	gold,
	helmet,
	light,
	offhand,
	ranged,
	ring,
	scroll_type, /* avoid conflict with ncurses */
	wand,
	weapon,
};

struct obj_description {
	dice			attr;
	std::vector<color>	colors;
	dice			dam;
	dice			def;
	std::string		desc;
	dice			dodge;
	dice			hit;
	std::string		name;
	int			rrty;
	dice			speed;
	type			obj_type;
	dice			val;
	dice			weight;

	bool			art;
};

extern std::vector<npc_description> npcs_parsed;
extern std::vector<obj_description> objs_parsed;

void	parse_npc_file();
void	parse_obj_file();

#endif /* PARSE_H */
