#ifndef PARSE_H
#define PARSE_H

#include <string>
#include <vector>

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

struct parser_npc {
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

extern std::vector<parser_npc> npcs_parsed;
extern parser_npc c_npc;

void	parse_dice(struct dice *const, char *const);
void	read_desc(std::string &);

enum color	parse_color(char const *const);
enum ability	parse_ability(char const *const);

#endif /* PARSE_H */
