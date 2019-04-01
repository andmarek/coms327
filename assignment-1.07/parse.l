%{
#include "parse.h"
#include "y.tab.h"
%}

%option noyywrap

S	[A-Za-z0-9+'\",]

COLORS	RED|GREEN|BLUE|CYAN|YELLOW|MAGENTA|WHITE|BLACK
ABILS	SMART|TELE|TUNNEL|ERRATIC|PASS|PICKUP|DESTROY|UNIQ|BOSS

%%

^"RLG327 MONSTER DESCRIPTION 1"$	return BEGIN_FILE;

^"BEGIN MONSTER"$	{ c_npc = {}; return BEGIN_MONSTER; }
^"END"$	{ npcs_parsed.push_back(c_npc); return END_MONSTER; }

^NAME	return NAME;
^SYMB	return SYMB;
^COLOR	return COLOR;
^SPEED	return SPEED;
^DAM	return DAM;
^HP	return HP;
^ABIL	return ABIL;
^RRTY	return RRTY;
^DESC	return DESC;

{COLORS}	{ yylval.str = yytext; return COLORS; }
{ABILS}		{ yylval.str = yytext; return ABILS; }

{S}+	{ yylval.str = yytext; return STR; }

.|\n	;

%%
