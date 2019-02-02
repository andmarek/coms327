#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DIRECTORY	"/.rlg327"
#define FILEPATH	"/dungeon"
#define LEN		8

#define FT	"RLG327-S2019"
#define FTLEN	12

int
save_dungeon(void) {
	struct stat st;
	char *home;
	int i;

#ifdef _GNU_SOURCE
	home = secure_getenv("HOME");
#else
	home = getenv("HOME");
#endif

	if (!home) {
		return -1;
	}

	strncat(home, DIRECTORY, LEN + 1);

	if (stat(home, &st) == -1) {
		if ((i = mkdir(home, 0700)) == -1) {
			return i;
		}
	}

	strncat(home, FILEPATH, LEN + 1);

	return 0;
}
