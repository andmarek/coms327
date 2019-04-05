%{
#include "parse.h"
#include "y.tab.h"

extern bool npc;
extern bool obj;
extern npc_description c_npc;
extern obj_description c_obj;
%}

%option nounput noyywrap

S	[[:graph:]]

COLORS	BLACK|BLUE|CYAN|GREEN|MAGENTA|RED|WHITE|YELLOW
ABILS	BOSS|DESTROY|ERRATIC|PASS|PICKUP|SMART|TELE|TUNNEL|UNIQ

TYPES_1	AMMUNITION|AMULET|ARMOR|BOOK|BOOTS|CLOAK|CONTAINER|FLASK|FOOD|GLOVES
TYPES_2	GOLD|HELMET|LIGHT|OFFHAND|RANGED|RING|SCROLL|WAND|WEAPON

TYPES	{TYPES_1}|{TYPES_2}

BOOLEAN	TRUE|FALSE

%Start desc
%%

^"RLG327 MONSTER DESCRIPTION 1"$	{
						npc = true;
						obj = false;
						return BEGIN_NPC_FILE;
					}
^"RLG327 OBJECT DESCRIPTION 1"$		{
						obj = true;
						npc = false;
						return BEGIN_OBJ_FILE;
					}

^"BEGIN MONSTER"$	{ c_npc = {}; return BEGIN_NPC; }
^"BEGIN OBJECT"$	{ c_obj = {}; return BEGIN_OBJ; }

^"END"$	{
		if (npc) npcs_parsed.push_back(c_npc);
		if (obj) objs_parsed.push_back(c_obj);
		return END;
	}

^COLOR	return COLOR;
^DAM	return DAM;
^NAME	return NAME;
^RRTY	return RRTY;
^SPEED	return SPEED;

^ABIL	return ABIL;
^HP	return HP;
^SYMB	return SYMB;

^ART	return ART;
^ATTR	return ATTR;
^DEF	return DEF;
^DODGE	return DODGE;
^HIT	return HIT;
^TYPE	return TYPE;
^VAL	return VAL;
^WEIGHT	return WEIGHT;

{ABILS}		{ yylval.str = yytext; return ABILS; }
{BOOLEAN}	{ yylval.str = yytext; return BOOLEAN; }
{COLORS}	{ yylval.str = yytext; return COLORS; }
{TYPES}		{ yylval.str = yytext; return TYPES; }

{S}+	{ yylval.str = yytext; return STR; }

^DESC\n		{ BEGIN desc; return DESC; }
<desc>^\.$	{ BEGIN 0; return DESC_END; }
<desc>.+\n|\n	{ yylval.str = yytext; return DESC_INNER; }

.|\n	;

%%
