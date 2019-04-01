#include <cstring>
#include <iostream>
#include <unordered_map>

#include "cerr.h"
#include "parse.h"
#include "y.tab.h"

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
	{"SCROLL", scroll},
	{"WAND", wand},
	{"WEAPON", weapon}
};

bool npc;
bool obj;

std::vector<parser_npc> npcs_parsed;
parser_npc c_npc;
std::vector<parser_obj> objs_parsed;
parser_obj c_obj;

constexpr static int const line_max = 77;

extern FILE	*yyin;

extern int	yyparse();
static void	print_dice(std::string const &, struct dice const &);
static void	print_npcs();
static void	print_objs();

int
main()
{
	int ret;

	yyin = fopen("monster_desc.txt", "r");

	if (yyin == NULL) {
		return 3;
	}

	ret = yyparse();

	if (ret != 0) {
		return ret;
	}

	if (npc) {
		print_npcs();
	}

	fclose(yyin);

	yyin = fopen("object_desc.txt", "r");

	if (yyin == NULL) {
		return 3;
	}

	ret = yyparse();

	if (ret != 0) {
		return ret;
	}

	if (obj) {
		print_objs();
	}

	return 0;
}

void
yyerror(char const *const s)
{
	cerrx(2, "%s", s);
}

void
parse_dice(struct dice *const d, char *const s)
{
	char *p, *last;

	p = strtok_r(s, "+", &last);

	if (p == NULL) {
		cerrx(2, "dice formatted incorrectly");
	}

	d->base = std::atoi(p);

	p = strtok_r(NULL, "d", &last);

	if (p == NULL) {
		cerrx(2, "dice formatted incorrectly");
	}

	d->dice = std::atoi(p);

	p = strtok_r(NULL, "\0", &last);

	if (p == NULL) {
		cerrx(2, "dice format missing '+'");
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
			cerrx(2, "desc line too long (%d)", ln.length());
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
	std::cout << name << ": (" << d.base << ", " << d.dice << ", "
		<< d.sides << ")\n";
}

static void
print_npcs()
{
	for (auto const &n: npcs_parsed) {
		std::cout << "name: " << n.name << '\n'
			<< "desc:\n" << n.desc << '\n'
			<< "colors: ";

		for(auto const &c : n.colors) {
			std::cout << c << ", ";
		}

		std::cout << '\n';

		print_dice("speed", n.speed);

		for(auto const &a: n.abils) {
			std::cout << a << ", ";
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
			<< "type: " << o.obj_type << '\n'
			<< "colors: ";

		for (auto const &c : o.colors) {
			std::cout << c << ", ";
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
			<< "art: " << o.art << "\n\n";
	}
}
