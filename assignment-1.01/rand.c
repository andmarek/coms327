/* TODO:
	avoid rand(): read from /dev/urandom
*/

#include<time.h>

#include "opal.h"

void
init_rand(char *str)
{
	unsigned int seed;

	if (str != NULL) {
		seed = hash(str);
	} else {
		seed = (unsigned int)time(NULL);
	}

	srand(seed);
}

/* biased, range inclusive */
int
rrand(int min, int max)
{
	return rand() % (max + 1 - min) + min;
}
