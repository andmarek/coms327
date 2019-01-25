/* TODO:
	seed from argument needs to be converted to uint
	avoid rand(): read from /dev/urandom, or alternatively:
	avoid time(): read from /dev/urandom
*/

#include <time.h>

#include "opal.h"

unsigned int
init_rand(char *str)
{
	unsigned int seed;

	if (str != NULL) {
		seed = hash(str);
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
