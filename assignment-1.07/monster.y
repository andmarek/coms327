%union {
	char *str;
}

%token BEGIN_FILE
%token BEGIN_MONSTER END_MONSTER
%token NAME SYMB COLOR DESC SPEED DAM HP ABIL RRTY

%token <str> ABILS
%token <str> COLORS
%token <str> STR

%type <str> abil
%type <str> color
%type <str> name

%%

file
	: BEGIN_FILE monsters
	;

monsters
	: monster
	| monsters monster
	;

monster
	: BEGIN_MONSTER keywords END_MONSTER
	;

keywords
	: keyword
	| keywords keyword
	;

keyword
	: NAME name
	| SYMB STR	{ c_npc.symb = $2[0]; }
	| COLOR color
	| DESC		{ read_desc(c_npc.desc); }
	| SPEED STR	{ parse_dice(&c_npc.speed, $2); }
	| DAM STR	{ parse_dice(&c_npc.dam, $2); }
	| HP STR	{ parse_dice(&c_npc.hp, $2); }
	| ABIL abil
	| RRTY STR	{ c_npc.rrty = atoi($2); }
	;

name
	: STR		{ c_npc.name += $1; }
	| name STR	{ c_npc.name = c_npc.name + " " + $2; }
	;

color
	: COLORS	{ c_npc.colors.push_back(parse_color($1)); }
	| color COLORS	{ c_npc.colors.push_back(parse_color($2)); }
	;

abil
	: ABILS		{ c_npc.abilities.push_back(parse_ability($1)); }
	| abil ABILS	{ c_npc.abilities.push_back(parse_ability($2)); }
	;

%%

/* TODO C++ style */

#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>

#include "cerr.h"
#include "monster.h"

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

std::vector<struct monster> npcs_parsed;
struct monster c_npc;

constexpr static int const line_max = 77;

extern int	yylex();
static void	parse_dice(struct dice *const, char *const);
static void	read_desc(std::string &);

static enum color	parse_color(char const *const);
static enum ability	parse_ability(char const *const);

void
yyerror(char const *const s)
{
	cerrx(2, "%s", s);
}

int
main()
{
	int ret = yyparse();

	for (auto const &n: npcs_parsed) {
		std::cout << "name: " << n.name << "\ndesc:\n" << n.desc
			<< "\ncolors: ";
		for(auto const &c : n.colors) {
			std::cout << c << ", ";
		}
		std::cout << "\nspeed: (" << n.speed.base << ", "
			<< n.speed.dice << ", " << n.speed.sides
			<< ")\nabilities: ";
		for(auto const &a: n.abilities) {
			std::cout << a << ", ";
		}
		std::cout << "\nhp: (" << n.hp.base << ", " << n.hp.dice << ", "
			<< n.hp.sides << ")\ndam: (" << n.dam.base << ", "
			<< n.dam.dice << ", " << n.dam.sides << ")\nsymb: "
			<< n.symb << '\n' << "rrty: " << n.rrty << '\n';
	}

	return ret;
}

static void
parse_dice(struct dice *const d, char *const s)
{
	char *p, *last;

	p = strtok_r(s, "+", &last);

	if (p == NULL) {
		yyerror("dice formatted incorrectly");
	}

	d->base = std::atoi(p);

	p = strtok_r(NULL, "d", &last);

	if (p == NULL) {
		yyerror("dice formatted incorrectly");
	}

	d->dice = std::atoi(p);

	p = strtok_r(NULL, "\0", &last);

	if (p == NULL) {
		yyerror("dice format missing '+'");
	}

	d->sides = std::atoi(p);
}

static void
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

static enum color
parse_color(char const *const s)
{
	return color_map.at(std::string(s));
}

static enum ability
parse_ability(char const *const s)
{
	return ability_map.at(std::string(s));
}
