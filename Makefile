build: main.c dungeon.h parse.o combat.o
	gcc -g main.c parse.o combat.o -Wall -o dungeon.exe

parse.o: parse.c dungeon.h
	gcc -g -c parse.c -Wall -o parse.o

combat.o: combat.c dungeon.h
	gcc -g -c combat.c -Wall -o combat.o
