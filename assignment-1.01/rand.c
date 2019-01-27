/* TODO:
	avoid rand(): read from /dev/urandom, or alternatively:
	avoid time(): read from /dev/urandom
*/

#include <errno.h>
#include <time.h>

#include "opal.h"

static unsigned int
uint_or_hash(char *str)
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
init_rand(char *str)
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
rrand(int min, int max)
{
	return rand() % (max + 1 - min) + min;
}
