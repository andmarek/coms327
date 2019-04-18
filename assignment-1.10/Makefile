pong: pong.c
	gcc -O3 -std=c89 pong.c -o pong.out -lncurses

pong99: pong.c
	gcc -O3 -std=c99 pong.c -o pong.out -lncurses

debug: pong.c
	gcc -Wall -Wextra -O3 -g -pedantic -std=c89 pong.c -o pong.out -lncurses

clean:
	rm -f *.out error vgcore.*
