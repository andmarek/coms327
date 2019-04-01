%{
#include "parse.h"
extern int	yylex();
extern void	yyerror(char const *const);
%}

%union {
	char *str;
}

%token BEGIN_FILE
%token BEGIN_MONSTER END_MONSTER
%token NAME SYMB COLOR DESC SPEED DAM HP ABIL RRTY

%token <str> ABILS
%token <str> COLORS
%token <str> STR

%%

file
	: BEGIN_FILE npcs
	;

npcs
	: npc
	| npcs npc
	;

npc
	: BEGIN_MONSTER npc_keywords END_MONSTER
	;

npc_keywords
	: npc_keyword
	| npc_keywords npc_keyword
	;

npc_keyword
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
