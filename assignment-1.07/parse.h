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

struct parser_npc {
	std::vector<ability>	abils;
	std::vector<color>	colors;
	struct dice		dam;
	std::string		desc;
	struct dice		hp;
	std::string		name;
	int			rrty;
	struct dice		speed;
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

struct parser_obj {
	struct dice		attr;
	std::vector<color>	colors;
	struct dice		dam;
	struct dice		def;
	std::string		desc;
	struct dice		dodge;
	struct dice		hit;
	std::string		name;
	int			rrty;
	struct dice		speed;
	type			obj_type;
	struct dice		val;
	struct dice		weight;

	bool			art;
};

extern bool npc;
extern bool obj;

extern std::vector<parser_npc> npcs_parsed;
extern parser_npc c_npc;

extern std::vector<parser_obj> objs_parsed;
extern parser_obj c_obj;

void	parse_dice(struct dice *const, char *const);
void	read_desc(std::string &);

enum color	parse_color(char const *const);
enum ability	parse_ability(char const *const);
enum type	parse_type(char const *const);
bool		parse_boolean(char const *const);

bool	parse();

#endif /* PARSE_H */
