#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024	/* Image size */
#define KSIZE 3		/* Kernel size */
#define CEIL 2		/* ceil(KSIZE/2) */

/* TODO: acc_x = ... and acc_y = ... depend on uninitialized values */
void sobel(int8_t const m[][SIZE], int8_t const k_x[][KSIZE],
	int8_t const k_y[][KSIZE], int8_t out[][SIZE]) {

	int8_t acc_x, acc_y;
	size_t r, c, j, i;

	for (r = 0; r < SIZE; ++r) {
		for (c = 0; c < SIZE; ++c) {
			acc_x = acc_y = 0;

			for (j = 0; j < KSIZE; ++j) {
				for (i = 0; i < KSIZE; ++i) {
					acc_x += m[r + j - CEIL][c + i - CEIL] * k_x[j][i];
					acc_y += m[r + j - CEIL][c + i - CEIL] * k_y[j][i];
				}
			}

			out[r][c] = (int8_t)sqrt((double)(acc_x * acc_x + acc_y * acc_y));
		}
	}
}

/* Do not modify write_pgm() or read_pgm() */
int write_pgm(char *file, void *image, uint32_t x, uint32_t y) {
	FILE *o;

	if (!(o = fopen(file, "w"))) {
		perror(file);

		return -1;
	}

	fprintf(o, "P5\n%u %u\n255\n", x, y);

	/* Assume input data is correctly formatted.	*
	 * There's no way to handle it, otherwise.		*/
	if (fwrite(image, 1, x * y, o) != (x * y)) {
		perror("fwrite");
		fclose(o);

		return -1;
	}

	fclose(o);

	return 0;
}

/* A better implementation of this function would read the image dimensions	*
 * from the input and allocate the storage, setting x and y so that the		*
 * user can determine the size of the file at runtime.	In order to			*
 * minimize complication, I've written this version to require the user to	*
 * know the size of the image in advance.									*/
int read_pgm(char *file, void *image, uint32_t x, uint32_t y) {
	FILE *f;
	char s[80];
	unsigned i, j;

	if (!(f = fopen(file, "r"))) {
		perror(file);

		return -1;
	}

	if (!fgets(s, 80, f) || strncmp(s, "P5", 2)) {
		fprintf(stderr, "Expected P6\n");

		return -1;
	}

	/* Eat comments */
	do {
		fgets(s, 80, f);
	} while (s[0] == '#');

	if (sscanf(s, "%u %u", &i, &j) != 2 || i != x || j != y) {
		fprintf(stderr, "Expected x and y dimensions %u %u\n", x, y);
		fclose(f);

		return -1;
	}

	/* Eat comments */
	do {
		fgets(s, 80, f);
	} while (s[0] == '#');

	if (strncmp(s, "255", 3)) {
		fprintf(stderr, "Expected 255\n");
		fclose(f);

		return -1;
	}

	if (fread(image, 1, x * y, f) != x * y) {
		perror("fread");
		fclose(f);

		return -1;
	}

	fclose(f);

	return 0;
}

int main(int argc, char *argv[]) {
	int8_t image[SIZE][SIZE];
	int8_t out[SIZE][SIZE];

	int8_t const k_x[KSIZE][KSIZE] = {
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	};

	int8_t const k_y[KSIZE][KSIZE] = {
		{-1, -2, -1},
		{0, 0, 0},
		{1, 2, 1}
	};

	if (argc != 2) {
		fprintf(stderr, "%s expects filename as only argument\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Example usage of PGM functions */
	/* This assumes that motorcycle.pgm is a pgm image of size SIZExSIZE */
	if (read_pgm(argv[1], image, SIZE, SIZE) == -1) {
		return EXIT_FAILURE;
	}

	sobel(image, k_x, k_y, out);

	/* After processing the image and storing your output in "out", write	*
	 * to motorcycle.edge.pgm.												*/
	if (write_pgm("sobel.pgm", out, SIZE, SIZE) == -1) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
