#include "opal.h"

/* djb2 hash algorithm */
unsigned int
hash(char *str)
{
	unsigned int hash = 5381;
	int c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + (unsigned char)c;
	}

	return hash;
}
