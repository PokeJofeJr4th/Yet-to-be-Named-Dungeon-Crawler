#include "dungeon.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct ExitP
{
    char *exit_to;
    struct ExitP *next;
    enum Direction exit_dir;
};

typedef struct EnemyP
{
    char *name;
    struct EnemyP *next;
};

typedef struct ItemP
{
    char *name;
    struct ItemP *next;
};

typedef struct RoomP
{
    char *name;
    char *desc;
    struct ExitP *exits;
    struct EnemyP *enemies;
    struct ItemP *items;
    struct RoomP *next;
};

typedef struct DungeonP
{
    struct RoomP *rooms;
};

char *fmt_dir(enum Direction d);
void print_dungeon(struct DungeonP d);

struct Dungeon *load_dungeon(char *filename)
{
    struct DungeonP d;
    int num_rooms = 0;
    d.rooms = 0;
    char l[128];
    FILE *f = fopen(filename, "r");

    while (fgets(l, 128, f))
    {
        for (int i = strlen(l) - 1; i > 0 && isspace(l[i]); l[i++] = '\0')
            ;
        char *line = l;
        while (isspace(*line++))
            ;
        line--;
        if (strncmp(line, "ROOM ", 5) == 0)
        {
            num_rooms++;
            struct RoomP *r = malloc(sizeof(struct RoomP));
            r->next = d.rooms;
            d.rooms = r;
            r->enemies = 0;
            r->exits = 0;
            r->items = 0;
            r->name = strdup(line + 5);
            r->desc = "";
        }
        else if (strncmp(line, "EXIT ", 5) == 0)
        {
            enum Direction dir;
            char *exit;
            if (strncmp(line + 5, "NORTH ", 6) == 0)
            {
                dir = NORTH;
                exit = strdup(line + 11);
            }
            else if (strncmp(line + 5, "SOUTH ", 6) == 0)
            {
                dir = SOUTH;
                exit = strdup(line + 11);
            }
            else if (strncmp(line + 5, "EAST ", 5) == 0)
            {
                dir = EAST;
                exit = strdup(line + 10);
            }
            else if (strncmp(line + 5, "WEST ", 5) == 0)
            {
                dir = WEST;
                exit = strdup(line + 10);
            }
            else
            {
                // TODO: error that you're trying to go in an invalid direction
                continue;
            }
            struct ExitP *e = malloc(sizeof(struct ExitP));
            e->exit_dir = dir;
            e->exit_to = exit;
            e->next = d.rooms->exits;
            d.rooms->exits = e;
        }
        else if (strncmp(line, "ITEM ", 5) == 0)
        {
            struct ItemP *i = malloc(sizeof(struct ItemP));
            i->name = strdup(line + 5);
            i->next = d.rooms->items;
            d.rooms->items = i;
        }
        else if (strncmp(line, "ENEMY ", 6) == 0)
        {
            struct EnemyP *e = malloc(sizeof(struct EnemyP));
            e->name = strdup(line + 6);
            e->next = d.rooms->enemies;
            d.rooms->enemies = e;
        }
        else if (strncmp(line, "DESC ", 5) == 0)
        {
            d.rooms->desc = strdup(line + 5);
        }
        // ignore empty/invalid lines?
    }
    fclose(f);
    struct Dungeon *dungeon = malloc(sizeof(struct Dungeon) + sizeof(struct Room *) * num_rooms);
    print_dungeon(d);
    return 0;
}

void print_dungeon(struct DungeonP d)
{
    for (struct RoomP *r = d.rooms; r != 0; r = r->next)
    {
        printf("\nROOM %s", r->name);
        if (*r->desc)
        {
            printf("\n DESC %s", r->desc);
        }
        for (struct EnemyP *e = r->enemies; e != 0; e = e->next)
        {
            printf("\n ENEMY %s", e->name);
        }
        for (struct ItemP *i = r->items; i != 0; i = i->next)
        {
            printf("\n ITEM %s", i->name);
        }
        for (struct ExitP *e = r->exits; e != 0; e = e->next)
        {
            printf("\n EXIT %s %s", fmt_dir(e->exit_dir), e->exit_to);
        }
    }
}

char *fmt_dir(enum Direction d)
{
    switch (d)
    {
    case NORTH:
        return "NORTH";
    case SOUTH:
        return "SOUTH";
    case EAST:
        return "EAST";
    case WEST:
        return "WEST";
    }
}
