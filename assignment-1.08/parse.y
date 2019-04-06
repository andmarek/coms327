%{
#include <cstring>
#include <unordered_map>

#include "cerr.h"
#include "globs.h"

extern int	yylex();

static void	yyerror(char const *const);

static bool	parse_boolean(char const *const);
static color	parse_color(char const *const);
static dice	parse_dice(char *const);
static uint8_t	parse_dice_value(char *const);
static type	parse_type(char const *const);

bool in_n;
bool in_o;

npc c_npc;
obj c_obj;

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

static std::unordered_map<std::string, uint16_t> const ability_map = {
	{"BOSS", BOSS},
	{"DESTROY", DESTROY},
	{"ERRATIC", ERRATIC},
	{"PASS", PASS},
	{"PICKUP", PICKUP},
	{"SMART", SMART},
	{"TELE", TELE},
	{"TUNNEL", TUNNEL},
	{"UNIQ", UNIQ}
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
	| DAM STR	{ c_obj.dam = parse_dice($2); }
	| DESC desc DESC_END
	| HP STR	{ c_npc.hp = parse_dice_value($2); }
	| NAME name
	| RRTY STR	{
				c_npc.rrty = (uint8_t)std::atoi($2);
				if (c_npc.rrty == 0 || c_npc.rrty > 100) {
					yyerror("rrty out of bounds [1, 100]");
				}
			}
	| SPEED STR	{ c_npc.speed = parse_dice_value($2); }
	| SYMB STR	{ c_npc.symb = $2[0]; }
	;

abil
	: ABILS		{ c_npc.type |= ability_map.at($1); }
	| abil ABILS	{ c_npc.type |= ability_map.at($2); }
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
	| ATTR STR	{ c_obj.attr = parse_dice_value($2); }
	| COLOR color
	| DAM STR	{ c_obj.dam = parse_dice($2); }
	| DEF STR	{ c_obj.def = parse_dice_value($2); }
	| DESC desc DESC_END
	| DODGE STR	{ c_obj.dodge = parse_dice_value($2); }
	| HIT STR	{ c_obj.hit = parse_dice_value($2); }
	| NAME name
	| RRTY STR	{
				c_obj.rrty = (uint8_t)std::atoi($2);
				if (c_obj.rrty == 0 || c_obj.rrty > 100) {
					yyerror("rrty out of bounds [1, 100]");
				}
			}
	| SPEED STR	{ c_obj.speed = parse_dice_value($2); }
	| TYPE TYPES	{ c_obj.obj_type = parse_type($2); }
	| VAL STR	{ c_obj.val = parse_dice_value($2); }
	| WEIGHT STR	{ c_obj.val = parse_dice_value($2); }
	;

name
	: STR		{
				if (in_n) c_npc.name += $1;
				if (in_o) c_obj.name += $1;
			}
	| name STR	{
				if (in_n) c_npc.name = c_npc.name + " " + $2;
				if (in_o) c_obj.name = c_obj.name + " " + $2;
			}
	;

color
	: COLORS	{
				if (in_n) c_npc.colors.push_back(parse_color($1));
				if (in_o) c_obj.colors.push_back(parse_color($1));
			}
	| color COLORS	{
				if (in_n) c_npc.colors.push_back(parse_color($2));
				if (in_o) c_obj.colors.push_back(parse_color($2));
			}
	;

desc
	: DESC_INNER		{
					if (in_n) c_npc.desc += $1;
					if (in_o) c_obj.desc += $1;
				}
	| desc DESC_INNER	{
					if (in_n) c_npc.desc += $2;
					if (in_o) c_obj.desc += $2;
				}
	;

%%

static void
yyerror(char const *const s)
{
	cerrx(1, "%s", s);
}


static bool
parse_boolean(char const *const s)
{
	return std::strcmp(s, "TRUE") == 0;
}

static color
parse_color(char const *const s)
{
	return color_map.at(s);
}

static dice
parse_dice(char *const s)
{
	dice d;
	char *p, *last;

	p = strtok_r(s, "+", &last);

	if (p == NULL) {
		cerrx(1, "dice formatted incorrectly");
	}

	d.base = (uint8_t)std::atoi(p);

	p = strtok_r(NULL, "d", &last);

	if (p == NULL) {
		cerrx(1, "dice formatted incorrectly");
	}

	d.dice = (uint8_t)std::atoi(p);

	p = strtok_r(NULL, "\0", &last);

	if (p == NULL) {
		cerrx(1, "dice format missing '+'");
	}

	d.sides = (uint8_t)std::atoi(p);

	return d;
}

static uint8_t
parse_dice_value(char *const s)
{
	uint8_t base, dice, sides;
	char *p, *last;

	p = strtok_r(s, "+", &last);

	if (p == NULL) {
		cerrx(1, "dice formatted incorrectly");
	}

	base = (uint8_t)std::atoi(p);

	p = strtok_r(NULL, "d", &last);

	if (p == NULL) {
		cerrx(1, "dice formatted incorrectly");
	}

	dice = (uint8_t)std::atoi(p);

	p = strtok_r(NULL, "\0", &last);

	if (p == NULL) {
		cerrx(1, "dice format missing '+'");
	}

	sides = (uint8_t)std::atoi(p);

	return rr.rand_dice<uint8_t>(base, dice, sides);
}

static type
parse_type(char const *const s)
{
	return type_map.at(std::string(s));
}

