build: main.c dungeon.h parse.o
	gcc -g main.c parse.o -Wall -o dungeon.exe

parse.o: parse.c dungeon.h
	gcc -g -c parse.c -Wall -o parse.o
