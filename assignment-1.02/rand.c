/* TODO:
	avoid rand(): read from /dev/urandom, or alternatively:
	avoid time(): read from /dev/urandom
*/

#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "hash.h"

static unsigned int
uint_or_hash(char const *const str)
{
	char *end;
	unsigned int ret;

	ret = (unsigned int)strtoul(str, &end, 10);

	if (str == end || errno == EINVAL || errno == ERANGE) {
		return hash(str);
	}

	return ret;
}

unsigned int
init_rand(char const *const str)
{
	unsigned int seed;

	if (str != NULL) {
		seed = uint_or_hash(str);
	} else {
		seed = (unsigned int)time(NULL);
	}

	srand(seed);

	return seed;
}

/* biased, range inclusive */
int
rrand(int const min, int const max)
{
	return rand() % (max + 1 - min) + min;
}
