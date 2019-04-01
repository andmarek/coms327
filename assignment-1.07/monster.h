#ifndef MONSTER_H
#define MONSTER_H

#include <vector>
#include <string>

enum color {
	red,
	green,
	blue,
	cyan,
	yellow,
	magenta,
	white,
	black
};

enum ability {
	smart,
	tele,
	tunnel,
	erratic,
	pass,
	pickup,
	destroy,
	uniq,
	boss
};

struct dice {
	int	base;
	int	dice;
	int	sides;
};

struct monster {
	std::string		name;
	std::string		desc;
	std::vector<color>	colors;
	struct dice		speed;
	std::vector<ability>	abilities;
	struct dice		hp;
	struct dice		dam;
	char			symb;
	int			rrty;
};

extern std::vector<struct monster> npcs_parsed;
extern struct monster c_npc;

#endif /* MONSTER_H */
