#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "player.h"
#include "opal.h"

#define DIRECTORY	"/.rlg327"
#define FILEPATH	"/dungeon"
#define DF_L		8

#define MARK	"RLG327-S2019"
#define MARK_L	12

static uint32_t const VER = 0;
static size_t const VER_L = sizeof(uint32_t);

// TODO endianess
static int
write_things(FILE *const f)
{
	/* type marker */
	if (fwrite(MARK, MARK_L, 1, f) != 1) {
		return -1;
	}

	/* version */
	if (fwrite(&VER, VER_L, 1, f) != 1) {
		return -1;
	}

	/* size */

	/* player coords */
	if (fwrite(&p, sizeof(struct player), 1, f) != 1) {
		return -1;
	}

	/* hardness */

	/* room num */
	if (fwrite(&room_count, sizeof(uint8_t), 1, f) != 1) {
		return -1;
	}

	/* room data */
	if (fwrite(rooms, sizeof(struct room), room_count, f) != room_count) {
		return -1;
	}

	/* stairs_up num */
	if (fwrite(&stair_up_count, sizeof(uint8_t), 1, f) != 1) {
		return -1;
	}

	/* stars_up coords */
	if (fwrite(stairs_up, sizeof(struct stair), stair_up_count, f) != stair_up_count) {
		return -1;
	}

	/* stairs_dn num */
	if (fwrite(&stair_dn_count, sizeof(uint8_t), 1, f) != 1) {
		return -1;
	}

	/* stairs_dn coords */
	if (fwrite(stairs_dn, sizeof(struct stair), stair_dn_count, f) != stair_dn_count) {
		return -1;
	}

	return 0;
}

int
save_dungeon(void)
{
	struct stat st;
	FILE *f;
	char *path;

#ifdef _GNU_SOURCE
	path = secure_getenv("HOME");
#else
	path = getenv("HOME");
#endif

	if (!path) {
		return -1;
	}

	strncat(path, DIRECTORY, DF_L + 1);

	if (stat(path, &st) == -1 && mkdir(path, 0700) == -1) {
		return -1;
	}

	strncat(path, FILEPATH, DF_L + 1);

	if (!(f = fopen(path, "w"))) {
		return -1;
	}

	return write_things(f);
}
