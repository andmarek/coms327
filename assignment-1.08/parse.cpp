#include <iostream>

#include <sys/stat.h>

#include "cerr.h"
#include "gen.h"
#include "parse.h"
#include "y.tab.h"

static char const *const NPC_FILE = "/monster_desc.txt";
static char const *const OBJ_FILE = "/object_desc.txt";

static char const *const color_map_r[] = {
	"BLACK",
	"BLUE",
	"CYAN",
	"GREEN",
	"MAGENTA",
	"RED",
	"WHITE",
	"YELLOW"
};

static char const *const ability_map_r[] = {
	"BOSS",
	"DESTROY",
	"ERRATIC",
	"PASS",
	"PICKUP",
	"SMART",
	"TELE",
	"TUNNEL",
	"UNIQ"
};

static char const *const type_map_r[] = {
	"AMMUNITION",
	"AMULET",
	"ARMOR",
	"BOOK",
	"BOOTS",
	"CLOAK",
	"CONTAINER",
	"FLASK",
	"FOOD",
	"GLOVES",
	"GOLD",
	"HELMET",
	"LIGHT",
	"OFFHAND",
	"RANGED",
	"RING",
	"SCROLL",
	"WAND",
	"WEAPON"
};

extern int	yyparse();
extern FILE	*yyin;

std::vector<npc_description> npcs_parsed;
std::vector<obj_description> objs_parsed;

constexpr static int const line_max = 77;

void
parse_npc_file()
{
	struct stat st;
	std::string const path = rlg_path() + NPC_FILE;

	if (stat(path.c_str(), &st) == -1) {
		if (errno == ENOENT) {
			cerr(1, "no monster file, run with '-m' to skip "
				"monster file parsing");
		} else {
			cerr(1, "stat monster file");
		}
	}

	yyin = fopen(path.c_str(), "r");

	if (yyin == NULL) {
		cerr(1, "monster file fopen");
	}

	if (yyparse() != 0) {
		cerrx(1, "yyparse npcs");
	}

	if (fclose(yyin) == EOF) {
		cerr(1, "monster fclose");
	}
}

void
parse_obj_file()
{
	struct stat st;
	std::string const path = rlg_path() + OBJ_FILE;

	if (stat(path.c_str(), &st) == -1) {
		if (errno == ENOENT) {
			cerr(1, "no object file, run with '-o' to skip object "
				"parsing");
		} else {
			cerr(1, "stat object file");
		}
	}

	yyin = fopen(path.c_str(), "r");

	if (yyin == NULL) {
		cerr(1, "object file fopen");
	}

	if (yyparse() != 0) {
		cerrx(1, "yyparse objs");
	}

	if (fclose(yyin) == EOF) {
		cerr(1, "object fclose");
	}
}
