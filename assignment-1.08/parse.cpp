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

static void	print_dice(std::string const &, dice const &);
static void	print_npcs();
static void	print_objs();

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

	std::cout << "<===== NPCs =====>\n\n";
	print_npcs();
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

	std::cout << "<===== OBJs =====>\n\n";
	print_objs();
}

static void
print_dice(std::string const &name, struct dice const &d)
{
	std::cout << name << ": " << d.base << "+" << d.dice << "d" << d.sides
		<< '\n';
}

static void
print_npcs()
{
	for (auto const &n: npcs_parsed) {
		std::cout << "name: " << n.name << '\n'
			<< "desc:\n" << n.desc
			<< "colors: ";

		for(auto const &c : n.colors) {
			std::cout << color_map_r[c] << ", ";
		}

		std::cout << '\n';

		print_dice("speed", n.speed);

		std::cout << "abil: ";

		for(auto const &a: n.abils) {
			std::cout << ability_map_r[a] << ", ";
		}

		std::cout << '\n';

		print_dice("hp", n.hp);
		print_dice("dam", n.dam);

		std::cout << "symb: " << n.symb << '\n'
			<< "rrty: " << n.rrty << "\n\n";
	}
}

static void
print_objs()
{
	for (auto const &o: objs_parsed) {
		std::cout << "name: " << o.name << '\n'
			<< "type: " << type_map_r[o.obj_type] << '\n'
			<< "colors: ";

		for (auto const &c : o.colors) {
			std::cout << color_map_r[c] << ", ";
		}

		std::cout << '\n';

		print_dice("weight", o.weight);
		print_dice("dam", o.dam);
		print_dice("attr", o.attr);
		print_dice("val", o.val);
		print_dice("dodge", o.dodge);
		print_dice("def", o.def);
		print_dice("speed", o.speed);

		std::cout << "desc:\n" << o.desc
			<< "rrty: " << o.rrty << '\n'
			<< "art: " << (o.art ? "true" : "false") << "\n\n";
	}
}
