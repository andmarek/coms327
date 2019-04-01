#include <cstring>
#include <iostream>
#include <unordered_map>

#include "cerr.h"
#include "parse.h"
#include "y.tab.h"

std::vector<parser_npc> npcs_parsed;
parser_npc c_npc;

constexpr static int const line_max = 77;

static std::unordered_map<std::string, enum color> const color_map = {
	{"RED", red},
	{"GREEN", green},
	{"BLUE", blue},
	{"CYAN", cyan},
	{"YELLOW", yellow},
	{"MAGENTA", magenta},
	{"WHITE", white},
	{"BLACK", black}
};

static std::unordered_map<std::string, enum ability> const ability_map = {
	{"SMART", smart},
	{"TELE", tele},
	{"TUNNEL", tunnel},
	{"ERRATIC", erratic},
	{"PASS", pass},
	{"PICKUP", pickup},
	{"DESTROY", destroy},
	{"UNIQ", uniq},
	{"BOSS", boss}
};

extern int	yyparse();
static void	print_npcs();

int
main()
{
	int ret = yyparse();

	print_npcs();

	return ret;
}

void
yyerror(char const *const s)
{
	cerrx(2, "%s", s);
}

void
parse_dice(struct dice *const d, char *const s)
{
	char *p, *last;

	p = strtok_r(s, "+", &last);

	if (p == NULL) {
		cerrx(2, "dice formatted incorrectly");
	}

	d->base = std::atoi(p);

	p = strtok_r(NULL, "d", &last);

	if (p == NULL) {
		cerrx(2, "dice formatted incorrectly");
	}

	d->dice = std::atoi(p);

	p = strtok_r(NULL, "\0", &last);

	if (p == NULL) {
		cerrx(2, "dice format missing '+'");
	}

	d->sides = std::atoi(p);
}

void
read_desc(std::string &desc)
{
	std::string line;

	while (std::getline(std::cin, line)) {
		if (line == ".") {
			break;
		} else if (line.length() > line_max) {
			cerrx(2, "desc line too long (%d)", line.length());
		}

		desc += line;
		desc.push_back('\n');
	}

	desc.pop_back();
}

enum color
parse_color(char const *const s)
{
	return color_map.at(std::string(s));
}

enum ability
parse_ability(char const *const s)
{
	return ability_map.at(std::string(s));
}

void
print_npcs(void)
{
	for (auto const &n: npcs_parsed) {
		std::cout << "name: " << n.name << '\n'
			<< "desc:\n" << n.desc << '\n'
			<< "colors: ";

		for(auto const &c : n.colors) {
			std::cout << c << ", ";
		}

		std::cout << "\nspeed: (" << n.speed.base << ", "
			<< n.speed.dice << ", " << n.speed.sides << ")\n"
			<< "abilities: ";

		for(auto const &a: n.abilities) {
			std::cout << a << ", ";
		}

		std::cout << "\nhp: (" << n.hp.base << ", " << n.hp.dice << ", "
			<< n.hp.sides << ")\n"
			<< "dam: (" << n.dam.base << ", " << n.dam.dice << ", "
			<< n.dam.sides << ")\n"
			<< "symb: " << n.symb << '\n'
			<< "rrty: " << n.rrty << '\n';
	}
}
