%{
#include "monster.h"
#include "y.tab.h"
%}

SP	[A-Za-z0-9+'\",]
WS	[ \t\v\n\f]

COLORS	(RED|GREEN|BLUE|CYAN|YELLOW|MAGENTA|WHITE|BLACK)
ABILS	(SMART|TELE|TUNNEL|ERRATIC|PASS|PICKUP|DESTROY|UNIQ|BOSS)

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

^DESC	{ read_desc(c_npc.desc); return DESC; }

{COLORS}	{ yylval.str = strdup(yytext); return COLORS; }
{ABILS}		{ yylval.str = strdup(yytext); return ABILS; }

{SP}+	{ yylval.str = strdup(yytext); return STR; }

{WS}+	;
.	;

%%

int
yywrap(void)
{
	return 1;
}
