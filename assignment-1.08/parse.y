%{
#include <cstring>
#include <unordered_map>

#include "cerr.h"
#include "parse.h"

extern int	yylex();

static void	yyerror(char const *const);

static ability	parse_ability(char const *const);
static bool	parse_boolean(char const *const);
static color	parse_color(char const *const);
static void	parse_dice(dice *const, char *const);
static type	parse_type(char const *const);

bool npc;
bool obj;

npc_description c_npc;
obj_description c_obj;

static std::unordered_map<std::string, color> const color_map = {
	{"BLACK", black},
	{"BLUE", blue},
	{"CYAN", cyan},
	{"GREEN", green},
	{"MAGENTA", magenta},
	{"RED", red},
	{"WHITE", white},
	{"YELLOW", yellow}
};

static std::unordered_map<std::string, ability> const ability_map = {
	{"BOSS", boss},
	{"DESTROY", destroy},
	{"ERRATIC", erratic},
	{"PASS", pass},
	{"PICKUP", pickup},
	{"SMART", smart},
	{"TELE", tele},
	{"TUNNEL", tunnel},
	{"UNIQ", uniq}
};

static std::unordered_map<std::string, type> const type_map = {
	{"AMMUNITION", ammunition},
	{"AMULET", amulet},
	{"ARMOR", armor},
	{"BOOK", book},
	{"BOOTS", boots},
	{"CLOAK", cloak},
	{"CONTAINER", container},
	{"FLASK", flask},
	{"FOOD", food},
	{"GLOVES", gloves},
	{"GOLD", gold},
	{"HELMET", helmet},
	{"LIGHT", light},
	{"OFFHAND", offhand},
	{"RANGED", ranged},
	{"RING", ring},
	{"SCROLL", scroll_type}, /* avoid conflict with ncurses */
	{"WAND", wand},
	{"WEAPON", weapon}
};

%}

%union {
	char *str;
}

%token BEGIN_NPC_FILE BEGIN_OBJ_FILE
%token BEGIN_NPC BEGIN_OBJ END
%token COLOR DAM DESC DESC_END NAME RRTY SPEED
%token ABIL HP SYMB
%token ART ATTR DEF DODGE HIT TYPE VAL WEIGHT

%token <str> ABILS
%token <str> BOOLEAN
%token <str> COLORS
%token <str> DESC_INNER
%token <str> STR
%token <str> TYPES

%%

file
	: BEGIN_NPC_FILE npcs
	| BEGIN_OBJ_FILE objs
	;

npcs
	: npc
	| npcs npc
	;

npc
	: BEGIN_NPC npc_keywords END
	;

npc_keywords
	: npc_keyword
	| npc_keywords npc_keyword
	;

npc_keyword
	: ABIL abil
	| COLOR color
	| DAM STR	{ parse_dice(&c_npc.dam, $2); }
	| DESC desc DESC_END
	| HP STR	{ parse_dice(&c_npc.hp, $2); }
	| NAME name
	| RRTY STR	{
				c_npc.rrty = atoi($2);
				if (c_npc.rrty < 1 || c_npc.rrty > 100) {
					yyerror("rrty out of bounds [1, 100]");
				}
			}
	| SPEED STR	{ parse_dice(&c_npc.speed, $2); }
	| SYMB STR	{ c_npc.symb = $2[0]; }
	;

abil
	: ABILS		{ c_npc.abils.push_back(parse_ability($1)); }
	| abil ABILS	{ c_npc.abils.push_back(parse_ability($2)); }
	;

objs
	: obj
	| objs obj
	;

obj
	: BEGIN_OBJ obj_keywords END
	;

obj_keywords
	: obj_keyword
	| obj_keywords obj_keyword
	;

obj_keyword
	: ART BOOLEAN	{ c_obj.art = parse_boolean($2); }
	| ATTR STR	{ parse_dice(&c_obj.attr, $2); }
	| COLOR color
	| DAM STR	{ parse_dice(&c_obj.dam, $2); }
	| DEF STR	{ parse_dice(&c_obj.def, $2); }
	| DESC desc DESC_END
	| DODGE STR	{ parse_dice(&c_obj.dodge, $2); }
	| HIT STR	{ parse_dice(&c_obj.hit, $2); }
	| NAME name
	| RRTY STR	{
				c_obj.rrty = atoi($2);
				if (c_obj.rrty < 1 || c_obj.rrty > 100) {
					yyerror("rrty out of bounds [1, 100]");
				}
			}
	| SPEED STR	{ parse_dice(&c_obj.speed, $2); }
	| TYPE TYPES	{ c_obj.obj_type = parse_type($2); }
	| VAL STR	{ parse_dice(&c_obj.val, $2); }
	| WEIGHT STR	{ parse_dice(&c_obj.weight, $2); }
	;

name
	: STR		{
				if (npc) c_npc.name += $1;
				if (obj) c_obj.name += $1;
			}
	| name STR	{
				if (npc) c_npc.name = c_npc.name + " " + $2;
				if (obj) c_obj.name = c_obj.name + " " + $2;
			}
	;

color
	: COLORS	{
				if (npc) c_npc.colors.push_back(parse_color($1));
				if (obj) c_obj.colors.push_back(parse_color($1));
			}
	| color COLORS	{
				if (npc) c_npc.colors.push_back(parse_color($2));
				if (obj) c_obj.colors.push_back(parse_color($2));
			}
	;

desc
	: DESC_INNER		{
					if (npc) c_npc.desc += $1;
					if (obj) c_obj.desc += $1;
				}
	| desc DESC_INNER	{
					if (npc) c_npc.desc += $2;
					if (obj) c_obj.desc += $2;
				}
	;

%%

static void
yyerror(char const *const s)
{
	cerrx(1, "%s", s);
}


static ability
parse_ability(char const *const s)
{
	return ability_map.at(std::string(s));
}

static bool
parse_boolean(char const *const s)
{
	return std::string(s) == "TRUE";
}

static color
parse_color(char const *const s)
{
	return color_map.at(std::string(s));
}

static void
parse_dice(dice *const d, char *const s)
{
	char *p, *last;

	p = strtok_r(s, "+", &last);

	if (p == NULL) {
		cerrx(1, "dice formatted incorrectly");
	}

	d->base = std::atoi(p);

	p = strtok_r(NULL, "d", &last);

	if (p == NULL) {
		cerrx(1, "dice formatted incorrectly");
	}

	d->dice = std::atoi(p);

	p = strtok_r(NULL, "\0", &last);

	if (p == NULL) {
		cerrx(1, "dice format missing '+'");
	}

	d->sides = std::atoi(p);
}

static type
parse_type(char const *const s)
{
	return type_map.at(std::string(s));
}

