#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <endian.h>
#undef _BSD_SOURCE
#undef _DEFAULT_SOURCE
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cerr.h"
#include "player.h"
#include "opal.h"

#define DIRECTORY	"/.rlg327"
#define FILEPATH	"/dungeon"
#define DF_L		8

#define MARK	"RLG327-S2019"
#define MARK_L	12

static int
write_things(FILE *const f, int const w, int const h)
{
	uint32_t const ver = htobe32(0);
	uint32_t const filesize = htobe32((uint32_t)(1708 + (room_count * 4)
		+ (stair_up_count * 2) + (stair_dn_count * 2)));
	int i, j;

	/* type marker */
	if (fwrite(MARK, MARK_L, 1, f) != 1) {
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
	if (fwrite(&p, sizeof(struct player), 1, f) != 1) {
		return -1;
	}

	/* hardness */
	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
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
	if (fwrite(rooms, sizeof(struct room), room_count, f) != room_count) {
		return -1;
	}

	/* stairs_up num */
	stair_up_count = htobe16(stair_up_count);
	if (fwrite(&stair_up_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	stair_up_count = be16toh(stair_up_count);

	/* stars_up coords */
	if (fwrite(stairs_up, sizeof(struct stair), stair_up_count, f) != stair_up_count) {
		return -1;
	}

	/* stairs_dn num */
	stair_dn_count = htobe16(stair_dn_count);
	if (fwrite(&stair_dn_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	stair_dn_count = be16toh(stair_dn_count);

	/* stairs_dn coords */
	if (fwrite(stairs_dn, sizeof(struct stair), stair_dn_count, f) != stair_dn_count) {
		return -1;
	}

	return 0;
}

int
save_dungeon(int const w, int const h)
{
	struct stat st;
	FILE *f;
	char *home, *path;
	int ret;

#ifdef _GNU_SOURCE
	home = secure_getenv("HOME");
#else
	home = getenv("HOME");
#endif

	if (home == NULL) {
		return -1;
	}

	if (!(path = malloc(sizeof(char) * (strlen(home) + 2*DF_L)))
		&& (strlen(home) + 2*DF_L) != 0) {
		cerr(1, "save malloc path");
	}

	(void)strncpy(path, home, strlen(home));
	(void)strncat(path, DIRECTORY, DF_L + 1);

	if (stat(path, &st) == -1) {
		if (errno == ENOENT && mkdir(path, 0700) == -1) {
			cerr(1, "save mkdir");
		} else {
			cerr(1, "save stat");
		}
	}

	(void)strncat(path, FILEPATH, DF_L + 1);

	if (!(f = fopen(path, "w"))) {
		cerr(1, "save fopen");
	}

	free(path);

	ret = write_things(f, w, h);

	if (fclose(f) == EOF) {
		cerr(1, "save fclose, (write_things=%d)", ret);
	}

	return ret;
}

static int
load_things(FILE *const f, int const w, int const h) {
	int i, j;

	/* skip type marker, version, and size */
	if (fseek(f, MARK_L + 2 * sizeof(uint32_t), SEEK_SET) == -1) {
		return -1;
	}

	/* player coords */
	if (fread(&p, sizeof(struct player), 1, f) != 1) {
		return -1;
	}

	/* hardness */
	for (i = 0; i < h; ++i) {
		for (j = 0; j < w; ++j) {
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

	if (!(rooms = malloc(sizeof(struct room) * room_count))
		&& room_count != 0) {
		cerr(1, "load malloc rooms");
	}

	/* room data */
	if (fread(rooms, sizeof(struct room), room_count, f) != room_count) {
		return -1;
	}

	/* stair_up num */
	if (fread(&stair_up_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	stair_up_count = be16toh(stair_up_count);

	if (!(stairs_up = malloc(sizeof(struct stair) * stair_up_count))
		&& stair_up_count != 0) {
		cerr(1, "load malloc stairs_up");
	}

	/* stair_up coords */
	if (fread(stairs_up, sizeof(struct stair), stair_up_count, f) != stair_up_count) {
		return -1;
	}

	/* stair_dn num */
	if (fread(&stair_dn_count, sizeof(uint16_t), 1, f) != 1) {
		return -1;
	}
	stair_dn_count = be16toh(stair_dn_count);

	if (!(stairs_dn = malloc(sizeof(struct stair) * stair_dn_count))
		&& stair_dn_count != 0) {
		cerr(1, "load malloc stairs_dn");
	}

	/* stair_dn_coords */
	if (fread(stairs_dn, sizeof(struct stair), stair_dn_count, f) != stair_dn_count) {
		return -1;
	}

	return 0;
}

int
load_dungeon(int const w, int const h)
{
	FILE *f;
	char *home, *path;
	int ret;

#ifdef _GNU_SOURCE
	home = secure_getenv("HOME");
#else
	home = getenv("HOME");
#endif

	if (home == NULL) {
		return -1;
	}

	if (!(path = malloc(sizeof(char) * (strlen(home) + 2*DF_L)))
		&& (strlen(home) + 2*DF_L) != 0) {
		cerr(1, "load malloc path");
	}

	(void)strncpy(path, home, strlen(home));
	(void)strncat(path, DIRECTORY, DF_L + 1);
	(void)strncat(path, FILEPATH, DF_L + 1);

	if (!(f = fopen(path, "r"))) {
		cerr(1, "load fopen");
	}

	free(path);

	ret = load_things(f, w, h);

	if (fclose(f) == EOF) {
		cerr(1, "load fclose, (load_things=%d)", ret);
	}

	return ret;
}
