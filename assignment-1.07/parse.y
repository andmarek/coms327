%{
#include "cerr.h"
#include "parse.h"
extern int	yylex();
extern void	yyerror(char const *const);
%}

%union {
	char *str;
}

%token BEGIN_MONSTER_FILE BEGIN_OBJECT_FILE
%token BEGIN_MONSTER BEGIN_OBJECT END
%token COLOR DAM DESC NAME RRTY SPEED
%token ABIL HP SYMB
%token ART ATTR DEF DODGE HIT TYPE VAL WEIGHT

%token <str> ABILS
%token <str> BOOLEAN
%token <str> COLORS
%token <str> STR
%token <str> TYPES

%%

files
	: file
	| files file

file
	: BEGIN_MONSTER_FILE npcs
	| BEGIN_OBJECT_FILE objs
	;

npcs
	: npc
	| npcs npc
	;

npc
	: BEGIN_MONSTER npc_keywords END
	;

npc_keywords
	: npc_keyword
	| npc_keywords npc_keyword
	;

npc_keyword
	: ABIL abil
	| COLOR color
	| DAM STR	{ parse_dice(&c_npc.dam, $2); }
	| DESC		{ read_desc(c_npc.desc); }
	| HP STR	{ parse_dice(&c_npc.hp, $2); }
	| NAME name
	| RRTY STR	{
				c_npc.rrty = atoi($2);
				if (c_npc.rrty < 1 || c_npc.rrty > 100) {
					cerrx(1, "rrty '%d' invalid: out of "
						"bounds [1, 100]", c_npc.rrty);
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
	: BEGIN_OBJECT obj_keywords END
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
	| DESC		{ read_desc(c_obj.desc); }
	| DODGE STR	{ parse_dice(&c_obj.dodge, $2); }
	| HIT STR	{ parse_dice(&c_obj.hit, $2); }
	| NAME name
	| RRTY STR	{
				c_obj.rrty = atoi($2);
				if (c_obj.rrty < 1 || c_obj.rrty > 100) {
					cerrx(1, "rrty '%d' invalid: out of "
						"bounds [1, 100]", c_obj.rrty);
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

%%
