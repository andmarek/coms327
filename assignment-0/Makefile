CC = gcc

CFLAGS := -g -lm
CFLAGS += -std=c99 -pedantic
CFLAGS += -Wextra -Wall -Wshadow -Wconversion

CFLAGS += -ftrapv -fverbose-asm
CFLAGS += -Wundef -Wcast-align=strict -Wstrict-prototypes -Wstrict-overflow=5
CFLAGS += -Wwrite-strings -Waggregate-return -Wcast-qual -Wformat=2
CFLAGS += -Wswitch-default -Wswitch-enum -Wunreachable-code

DIRTY = *.out *.pgm vgcore.*

sobel: sobel.c
	$(CC) $(CFLAGS) -o sobel.out sobel.c

clean:
	rm -f $(DIRTY)

val:
	valgrind --max-stackframe=3000000 --track-origins=yes ./sobel.out motorcycle.pgm
