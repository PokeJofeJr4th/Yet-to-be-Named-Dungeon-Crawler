#include "dungeon.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct ExitTmp
{
    char *exit_to;
    struct ExitTmp *next;
    enum Direction exit_dir;
};

struct EnemyTmp
{
    char *name;
    int hp;
    struct EnemyTmp *next;
};

struct ItemTmp
{
    char *name;
    struct ItemTmp *next;
};

struct TagTmp
{
    char *tag;
    struct TagTmp *next;
};

struct RoomTmp
{
    char *name;
    char *desc;
    int num_exits;
    int num_tags;
    struct ExitTmp *exits;
    struct EnemyTmp *enemies;
    struct ItemTmp *items;
    struct TagTmp *tags;
    struct RoomTmp *next;
};

struct DungeonTmp
{
    struct RoomTmp *rooms;
};

void print_dungeon_tmp(struct DungeonTmp);
void print_dungeon(struct Dungeon *);

char *trim_wspace(char *p)
{
    while (isspace(*p++))
        ;
    return --p;
}

struct Dungeon *load_dungeon(char *filename)
{
    struct DungeonTmp dungeon_tmp;
    int num_rooms = 0;
    dungeon_tmp.rooms = 0;
    char line_buffer[128];
    FILE *f = fopen(filename, "r");
    struct EnemyTmp *current_enemy = 0;

    while (fgets(line_buffer, 128, f))
    {
        for (int i = strlen(line_buffer) - 1; i > 0 && isspace(line_buffer[i]); line_buffer[i--] = '\0')
            ;
        char *line = trim_wspace(line_buffer);
        if (strncmp(line, "ROOM ", 5) == 0)
        {
            // starting a new room
            num_rooms++;
            // allocate space for the new room
            struct RoomTmp *room_tmp = malloc(sizeof(struct RoomTmp));
            // hook it up to the linked list
            room_tmp->next = dungeon_tmp.rooms;
            dungeon_tmp.rooms = room_tmp;
            // initialize all the other fields
            room_tmp->num_exits = 0;
            room_tmp->num_tags = 0;
            room_tmp->enemies = 0;
            room_tmp->exits = 0;
            room_tmp->items = 0;
            room_tmp->tags = 0;
            // copy the name over
            room_tmp->name = strdup(trim_wspace(line + 5));
            // set an empty description
            room_tmp->desc = "";
        }
        else if (strncmp(line, "EXIT ", 5) == 0)
        {
            enum Direction dir;
            char *exit;
            line = trim_wspace(line + 5);
            if (strncmp(line, "NORTH ", 6) == 0)
            {
                dir = NORTH;
                exit = strdup(trim_wspace(line + 6));
            }
            else if (strncmp(line, "SOUTH ", 6) == 0)
            {
                dir = SOUTH;
                exit = strdup(trim_wspace(line + 6));
            }
            else if (strncmp(line, "EAST ", 5) == 0)
            {
                dir = EAST;
                exit = strdup(trim_wspace(line + 5));
            }
            else if (strncmp(line, "WEST ", 5) == 0)
            {
                dir = WEST;
                exit = strdup(trim_wspace(line + 5));
            }
            else
            {
                // TODO: error that you're trying to go in an invalid direction
                continue;
            }
            struct ExitTmp *exit_tmp = malloc(sizeof(struct ExitTmp));
            exit_tmp->exit_dir = dir;
            exit_tmp->exit_to = exit;
            exit_tmp->next = dungeon_tmp.rooms->exits;
            dungeon_tmp.rooms->exits = exit_tmp;
            dungeon_tmp.rooms->num_exits++;
        }
        else if (strncmp(line, "ITEM ", 5) == 0)
        {
            struct ItemTmp *item_tmp = malloc(sizeof(struct ItemTmp));
            // copy the item name
            item_tmp->name = strdup(trim_wspace(line + 5));
            // hook up to the linked list
            item_tmp->next = dungeon_tmp.rooms->items;
            dungeon_tmp.rooms->items = item_tmp;
        }
        else if (strncmp(line, "ENEMY ", 6) == 0)
        {
            struct EnemyTmp *enemy_tmp = malloc(sizeof(struct EnemyTmp));
            // copy the enemy name
            enemy_tmp->name = strdup(trim_wspace(line + 6));
            // hook up to the linked list
            enemy_tmp->next = dungeon_tmp.rooms->enemies;
            dungeon_tmp.rooms->enemies = enemy_tmp;
            current_enemy = enemy_tmp;
        }
        else if (strncmp(line, "HP ", 3) == 0)
        {
            int hp = -1;
            sscanf(trim_wspace(line + 3), "%i", &hp);
            if (hp != -1)
            {
                current_enemy->hp = hp;
            }
        }
        else if (strncmp(line, "DESC ", 5) == 0)
        {
            // add a description to the current room
            dungeon_tmp.rooms->desc = strdup(trim_wspace(line + 5));
        }
        else if (strncmp(line, "TAG ", 4) == 0)
        {
            struct TagTmp *tag_tmp = malloc(sizeof(struct TagTmp));
            // copy the tag name
            tag_tmp->tag = strdup(trim_wspace(line + 4));
            // hook up to the linked list
            tag_tmp->next = dungeon_tmp.rooms->tags;
            dungeon_tmp.rooms->tags = tag_tmp;
            dungeon_tmp.rooms->num_tags++;
        }
        // ignore empty/invalid lines (?)
    }
    fclose(f);
    // print_dungeon_tmp(dungeon_tmp);
    // allocate the dungeon with enough space to store all the rooms
    struct Dungeon *dungeon = malloc(sizeof(struct Dungeon) + sizeof(struct Room) * num_rooms);
    dungeon->num_rooms = num_rooms;
    // loop through the rooms of the tmp and final dungeon in parallel
    struct Room *room = dungeon->rooms;
    for (struct RoomTmp *room_tmp = dungeon_tmp.rooms; room_tmp != 0; room_tmp = room_tmp->next)
    {
        // start with an empty list
        room->enemies = 0;
        room->items = 0;
        // move over the name and description
        room->name = room_tmp->name;
        room->desc = room_tmp->desc;
        // move over the enemies
        for (struct EnemyTmp *enemy_tmp = room_tmp->enemies; enemy_tmp != 0; enemy_tmp = enemy_tmp->next)
        {
            struct Enemy *enemy = malloc(sizeof(struct Enemy));
            enemy->name = enemy_tmp->name;
            enemy->hp = enemy_tmp->hp;
            enemy->next = room->enemies;
            room->enemies = enemy;
        }
        // move over the exits
        room->num_exits = room_tmp->num_exits;
        room->exits = malloc(sizeof(struct Exit) * room->num_exits);
        struct Exit *exit = room->exits;
        for (struct ExitTmp *exit_tmp = room_tmp->exits; exit_tmp != 0; exit_tmp = exit_tmp->next)
        {
            exit->dir = exit_tmp->exit_dir;
            // default room in case something goes wrong
            exit->room = -1;
            // find the ID of the room the exit is going to
            int i = 0;
            for (struct RoomTmp *room_candidate = dungeon_tmp.rooms; room_candidate != 0; room_candidate = room_candidate->next)
            {
                if (strcmp(exit_tmp->exit_to, room_candidate->name) == 0)
                {
                    exit->room = i;
                    break;
                }
                i++;
            }
            if (exit->room == -1)
            {
                printf("Can't find room %s\n", exit_tmp->exit_to);
            }
            exit++;
        }
        // move over the items
        for (struct ItemTmp *item_tmp = room_tmp->items; item_tmp != 0; item_tmp = item_tmp->next)
        {
            struct Item *item = malloc(sizeof(struct Item));
            item->name = item_tmp->name;
            item->next = room->items;
            room->items = item;
        }
        // move over the tags
        room->num_tags = room_tmp->num_tags;
        room->tags = malloc(sizeof(char *) * room->num_tags);
        char **tag = room->tags;
        for (struct TagTmp *tag_tmp = room_tmp->tags; tag_tmp != 0; tag_tmp = tag_tmp->next)
        {
            *tag = tag_tmp->tag;
            tag++;
        }
        // go to the next room in the array
        room++;
    }
    for (struct RoomTmp *room = dungeon_tmp.rooms; room != 0;)
    {
        for (struct ExitTmp *exit = room->exits; exit != 0;)
        {
            struct ExitTmp *nxt = exit->next;
            free(exit->exit_to);
            free(exit);
            exit = nxt;
        }
        for (struct EnemyTmp *enemy = room->enemies; enemy != 0;)
        {
            struct EnemyTmp *nxt = enemy->next;
            free(enemy);
            enemy = nxt;
        }
        for (struct ItemTmp *item = room->items; item != 0;)
        {
            struct ItemTmp *nxt = item->next;
            free(item);
            item = nxt;
        }
        for (struct TagTmp *tag = room->tags; tag != 0;)
        {
            struct TagTmp *nxt = tag->next;
            free(tag);
            tag = nxt;
        }
        struct RoomTmp *r = room->next;
        free(room);
        room = r;
    }
    // print_dungeon(dungeon);
    return dungeon;
}

void print_dungeon_tmp(struct DungeonTmp dungeon)
{
    for (struct RoomTmp *room = dungeon.rooms; room != 0; room = room->next)
    {
        printf("\nROOM %s\n", room->name);
        if (*room->desc)
        {
            printf(" DESC %s\n", room->desc);
        }
        for (struct TagTmp *tag = room->tags; tag != 0; tag = tag->next)
        {
            printf(" TAG %s\n", tag->tag);
        }
        for (struct EnemyTmp *enemy = room->enemies; enemy != 0; enemy = enemy->next)
        {
            printf(" ENEMY %s\n  HP %u\n", enemy->name, enemy->hp);
        }
        for (struct ItemTmp *item = room->items; item != 0; item = item->next)
        {
            printf(" ITEM %s\n", item->name);
        }
        for (struct ExitTmp *exit = room->exits; exit != 0; exit = exit->next)
        {
            printf(" EXIT %s %s\n", fmt_dir(exit->exit_dir), exit->exit_to);
        }
    }
}

void print_dungeon(struct Dungeon *dungeon)
{
    for (int room_id = 0; room_id < dungeon->num_rooms; room_id++)
    {
        struct Room *room = &dungeon->rooms[room_id];
        printf("\nROOM %s\n", room->name);
        if (*room->desc)
        {
            printf(" DESC %s\n", room->desc);
        }
        for (int tag_id = 0; tag_id < room->num_tags; tag_id++)
        {
            printf(" TAG %s\n", room->tags[tag_id]);
        }
        for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
        {
            printf(" ENEMY %s\n", enemy->name);
        }
        for (struct Item *item = room->items; item != 0; item = item->next)
        {
            printf(" ITEM %s\n", item->name);
        }
        for (int exit_id = 0; exit_id < room->num_exits; exit_id++)
        {
            struct Exit *exit = &room->exits[exit_id];
            printf(" EXIT %s %s\n", fmt_dir(exit->dir), dungeon->rooms[exit->room].name);
        }
    }
}

char *fmt_dir(enum Direction dir)
{
    switch (dir)
    {
    case NORTH:
        return "NORTH";
    case SOUTH:
        return "SOUTH";
    case EAST:
        return "EAST";
    case WEST:
        return "WEST";
    default:
        return "ERROR";
    }
}
