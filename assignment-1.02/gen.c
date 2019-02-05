#include <stdlib.h>

#include "opal.h"

int
gen(void)
{
	if (!(rooms = malloc(sizeof(struct room) * room_count))) {
		return -1;
	}

	if (!(stairs_up = malloc(sizeof(struct stair) * stair_up_count))) {
		return -1;
	}

	if (!(stairs_dn = malloc(sizeof(struct stair) * stair_dn_count))) {
		return -1;
	}

	if (!(tiles = malloc(sizeof(struct tile) * (size_t)width * (size_t)height))) {
		return -1;
	}

	return 0;
}
