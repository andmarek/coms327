#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <endian.h>
#undef _BSD_SOURCE
#undef _DEFAULT_SOURCE

#include <sys/stat.h>

#include <iostream>
#include <string>

#include "cerr.h"
#include "globs.h"

static std::string const DIRECTORY = "/.rlg327";
static std::string const FILEPATH = "/dungeon";
static int constexpr DF_L = 8;

static std::string const MARK = "RLG327-S2019";
static int constexpr MARK_L = 12;

static int
write_things(FILE *const f)
{
	uint32_t const ver = htobe32(0);
	uint32_t const filesize = htobe32((uint32_t)(1708 + (room_count * 4)
		+ (stair_up_count * 2) + (stair_dn_count * 2)));

	/* type marker */
	if (fwrite(MARK.c_str(), MARK_L, 1, f) != 1) {
		return -1;
	}

	/* version */
	if (fwrite(&ver, sizeof(uint32_t), 1, f) != 1) {
		return -1;
	}

	/* size */
	if (fwrite(&filesize, sizeof(uint32_t), 1, f) != 1) {
		return -1;
	}

	/* player coords */
	if (fwrite(&player.x, sizeof(uint8_t), 1, f) != 1) {
		return -1;
	}
	if (fwrite(&player.y, sizeof(uint8_t), 1, f) != 1) {
		return -1;
	}

	/* hardness */
	for (std::size_t i = 0; i < HEIGHT; ++i) {
		for (std::size_t j = 0; j < WIDTH; ++j) {
			if (fwrite(&tiles[i][j].h, sizeof(uint8_t), 1, f) != 1) {
				return -1;
			}
		}
	}

	/* room num */
	room_count = htobe16(room_count);
	if (fwrite(&room_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	room_count = be16toh(room_count);

	/* room data */
	for (auto const &r : rooms) {
		if (fwrite(&r, sizeof(room), 1, f) != 1) {
			return -1;
		}
	}

	/* stairs_up num */
	stair_up_count = htobe16(stair_up_count);
	if (fwrite(&stair_up_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	stair_up_count = be16toh(stair_up_count);

	/* stars_up coords */
	for (auto const &s : stairs_up) {
		if (fwrite(&s, sizeof(stair), 1, f) != 1) {
			return -1;
		}
	}

	/* stairs_dn num */
	stair_dn_count = htobe16(stair_dn_count);
	if (fwrite(&stair_dn_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	stair_dn_count = be16toh(stair_dn_count);

	/* stairs_dn coords */
	for (auto const &s : stairs_dn) {
		if (fwrite(&s, sizeof(stair), 1, f) != 1) {
			return -1;
		}
	}

	return 0;
}

int
save_dungeon()
{
	struct stat st;
	FILE *f;
	char *home;
	int ret;
	std::string path;

#ifdef _GNU_SOURCE
	home = secure_getenv("HOME");
#else
	home = getenv("HOME");
#endif

	if (home == NULL) {
		return -1;
	}

	path = std::string(home) + DIRECTORY;

	if (stat(path.c_str(), &st) == -1) {
		if (errno == ENOENT) {
			if (mkdir(path.c_str(), 0700) == -1) {
				cerr(1, "save mkdir");
			}
		} else {
			cerr(1, "save stat");
		}
	}

	path += FILEPATH;

	if (!(f = fopen(path.c_str(), "w"))) {
		cerr(1, "save fopen");
	}

	ret = write_things(f);

	if (fclose(f) == EOF) {
		cerr(1, "save fclose, (write_things=%d)", ret);
	}

	return ret;
}

static int
load_things(FILE *const f) {
	/* skip type marker, version, and size */
	if (fseek(f, MARK_L + 2 * sizeof(uint32_t), SEEK_SET) == -1) {
		return -1;
	}

	/* player coords */
	if (fread(&player.x, sizeof(uint8_t), 1, f) != 1) {
		return -1;
	}
	if (fread(&player.y, sizeof(uint8_t), 1, f) != 1) {
		return -1;
	}

	/* hardness */
	for (std::size_t i = 0; i < HEIGHT; ++i) {
		for (std::size_t j = 0; j < WIDTH; ++j) {
			if (fread(&tiles[i][j].h, sizeof(uint8_t), 1, f) != 1) {
				return -1;
			}
		}
	}

	/* room num */
	if (fread(&room_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	room_count = be16toh(room_count);

	rooms.resize(room_count);

	/* room data */
	for (auto &r : rooms) {
		if (fread(&r, sizeof(room), 1, f) != 1) {
			return -1;
		}
	}

	/* stair_up num */
	if (fread(&stair_up_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	stair_up_count = be16toh(stair_up_count);

	stairs_up.resize(stair_up_count);

	/* stair_up coords */
	for (auto &s : stairs_up) {
		if (fread(&s, sizeof(stair), 1, f) != 1) {
			return -1;
		}
	}

	/* stair_dn num */
	if (fread(&stair_dn_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	stair_dn_count = be16toh(stair_dn_count);

	stairs_dn.resize(stair_dn_count);

	/* stair_dn_coords */
	for (auto &s : stairs_dn) {
		if (fread(&s, sizeof(stair), 1, f) != 1) {
			return -1;
		}
	}

	return 0;
}

int
load_dungeon()
{
	FILE *f;
	char *home;
	int ret;
	std::string path;

#ifdef _GNU_SOURCE
	home = secure_getenv("HOME");
#else
	home = getenv("HOME");
#endif

	if (home == NULL) {
		return -1;
	}

	path = std::string(home) + DIRECTORY + FILEPATH;

	if (!(f = fopen(path.c_str(), "r"))) {
		cerr(1, "load fopen");
	}

	ret = load_things(f);

	if (fclose(f) == EOF) {
		cerr(1, "load fclose, (load_things=%d)", ret);
	}

	return ret;
}
