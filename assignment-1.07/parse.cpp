#include <cstring>
#include <iostream>
#include <unordered_map>

#include <sys/stat.h>

#include "cerr.h"
#include "gen.h"
#include "parse.h"
#include "y.tab.h"

static char const *const MONSTER_FILE = "/monster_desc.txt";
static char const *const OBJECT_FILE = "/object_desc.txt";

static std::unordered_map<std::string, enum color> const color_map = {
	{"BLACK", black},
	{"BLUE", blue},
	{"CYAN", cyan},
	{"GREEN", green},
	{"MAGENTA", magenta},
	{"RED", red},
	{"WHITE", white},
	{"YELLOW", yellow}
};

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

static std::unordered_map<std::string, enum ability> const ability_map = {
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

static std::unordered_map<std::string, enum type> const type_map = {
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

static void	print_dice(std::string const &, struct dice const &);
static void	print_npcs();
static void	print_objs();

extern int	yyparse();
extern FILE	*yyin;

std::vector<parser_npc> npcs_parsed;
parser_npc c_npc;
std::vector<parser_obj> objs_parsed;
parser_obj c_obj;

bool npc;
bool obj;

constexpr static int const line_max = 77;

void
yyerror(char const *const s)
{
	cerrx(1, "%s", s);
}

void
parse_monster()
{
	struct stat st;
	std::string const path = rlg_path() + MONSTER_FILE;

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
		cerrx(1, "yyparse monster");
	}

	if (fclose(yyin) == EOF) {
		cerr(1, "monster fclose");
	}

	std::cout << "<===== NPCs =====>\n\n";
	print_npcs();
}

void
parse_object()
{
	struct stat st;
	std::string const path = rlg_path() + OBJECT_FILE;

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
		cerrx(1, "yyparse object");
	}

	if (fclose(yyin) == EOF) {
		cerr(1, "object fclose");
	}

	std::cout << "<===== OBJs =====>\n\n";
	print_objs();
}

void
parse_dice(struct dice *const d, char *const s)
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

void
read_desc(std::string &desc)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, yyin)) != -1) {
		std::string ln = line;

		if (ln == ".\n") {
			break;
		} else if (ln.length() > line_max + 1) {
			cerrx(1, "desc line too long (%d)", ln.length());
		}

		desc += ln;
	}

	free(line);

	desc.pop_back();
}

enum color
parse_color(char const *const s)
{
	return color_map.at(std::string(s));
}

enum ability
parse_ability(char const *const s)
{
	return ability_map.at(std::string(s));
}

enum type
parse_type(char const *const s)
{
	return type_map.at(std::string(s));
}

bool
parse_boolean(char const *const s)
{
	return std::string(s) == "TRUE";
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
			<< "desc:\n" << n.desc << '\n'
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

		std::cout << "desc:\n" << o.desc << '\n'
			<< "rrty: " << o.rrty << '\n'
			<< "art: " << (o.art ? "true" : "false") << "\n\n";
	}
}
