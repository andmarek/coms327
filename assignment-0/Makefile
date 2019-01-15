f := -ftrapv -fverbose-asm -finstrument-functions -Wextra -Wall -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wconversion -Wunreachable-code

b:
	gcc $(f) -g -lm -o sobel.out sobel.c

c:
	rm *.out vgcore.* *.edge.pgm sobel.pgm

e:
	vim sobel.c

s:
	splint sobel.c

r:
	./sobel.out motorcycle.pgm
	xdg-open ./sobel.pgm
d:
	gdb ./sobel.out

v:
	valgrind --max-stackframe=2097208 --track-origins=yes ./sobel.out motorcycle.pgm
