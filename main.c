#include "dungeon.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

void read_input(char *buffer)
{
    printf("\n> ");
    fgets(buffer, 128, stdin);
    for (int i = strlen(buffer) - 1; i > 0 && isspace(buffer[i]); buffer[i--] = '\0')
        ;
    printf("\n");
    // printf("%s\n", buffer);
}

int main()
{
    struct Dungeon *dungeon = load_dungeon("example.txt");
    struct Item *inventory = 0;
    struct Room *room = 0;
    struct Combatant player;
    player.atk = 1;
    player.hp = 10;
    player.def = 0;
    strcpy(player.name, "You");
    for (int i = 0; i < dungeon->num_rooms; i++)
    {
        struct Room *r = &dungeon->rooms[i];
        for (int t = 0; t < r->num_tags; t++)
        {
            if (strcmp(r->tags[t], "START") == 0)
            {
                room = r;
                break;
            }
        }
        if (room != 0)
            break;
    }
    char cmd_buffer[128];
    while (1)
    {
        printf("%s\n %s\n", room->name, room->desc);
        if (room->enemies != 0)
        {
            printf("Enemies:\n");
            for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
            {
                printf(" %s (%u HP, %u ATK, %u DEF)\n", enemy->stats.name, enemy->stats.hp, enemy->stats.atk, enemy->stats.def);
            }
        }
        if (room->items != 0)
        {
            printf("Items:\n");
            for (struct Item *item = room->items; item != 0; item = item->next)
            {
                printf(" %s\n", item->name);
            }
        }
        if (room->num_exits != 0)
        {
            printf("Exits:\n");
            for (int i = 0; i < room->num_exits; i++)
            {
                printf(" %s: %s\n", fmt_dir(room->exits[i].dir), dungeon->rooms[room->exits[i].room].name);
            }
        }
        printf("\n");
        for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
        {
            fight(&enemy->stats, &player);
        }
        printf("\n");
        if (inventory != 0)
        {
            printf("Inventory:\n");
            for (struct Item *item = inventory; item != 0; item = item->next)
            {
                printf(" %s\n", item->name);
            }
        }
        read_input(cmd_buffer);
        if (strncmp(cmd_buffer, "move ", 5) == 0)
        {
            enum Direction dir;
            if (stricmp(cmd_buffer + 5, "north") == 0)
            {
                dir = NORTH;
            }
            else if (stricmp(cmd_buffer + 5, "south") == 0)
            {
                dir = SOUTH;
            }
            else if (stricmp(cmd_buffer + 5, "east") == 0)
            {
                dir = EAST;
            }
            else if (stricmp(cmd_buffer + 5, "west") == 0)
            {
                dir = WEST;
            }
            else
            {
                printf("Unknown direction\n");
                continue;
            }
            int new_room = -1;
            for (int i = 0; i < room->num_exits; i++)
            {
                struct Exit e = room->exits[i];
                if (e.dir == dir)
                {
                    new_room = room->exits[i].room;
                    break;
                }
            }
            if (new_room == -1)
            {
                printf("Invalid exit\n");
            }
            else
            {
                room = &dungeon->rooms[new_room];
            }
        }
        else if (strncmp(cmd_buffer, "take ", 5) == 0)
        {
            char *item_name = trim_wspace(cmd_buffer + 5);
            struct Item *prev = 0;
            for (struct Item *item = room->items; item != 0; item = item->next)
            {
                if (strcmp(item_name, item->name) == 0)
                {
                    if (prev == 0)
                    {
                        room->items = item->next;
                    }
                    else
                    {
                        prev->next = item->next;
                    }
                    item->next = inventory;
                    inventory = item;
                    break;
                }
                prev = item;
            }
        }
        else if (strncmp(cmd_buffer, "drop ", 5) == 0)
        {
            char *item_name = trim_wspace(cmd_buffer + 5);
            struct Item *prev = 0;
            for (struct Item *item = inventory; item != 0; item = item->next)
            {
                if (strcmp(item_name, item->name) == 0)
                {
                    if (prev == 0)
                    {
                        inventory = item->next;
                    }
                    else
                    {
                        prev->next = item->next;
                    }
                    item->next = room->items;
                    room->items = item;
                    break;
                }
                prev = item;
            }
        }
        else if (strncmp(cmd_buffer, "fight ", 5) == 0)
        {
            char *enemy_name = trim_wspace(cmd_buffer + 5);
            struct Enemy *prev = 0;
            for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
            {
                if (strcmp(enemy->stats.name, enemy_name) == 0)
                {
                    fight(&player, &enemy->stats);
                    if (enemy->stats.hp <= 0)
                    {
                        if (prev == 0)
                        {
                            room->enemies = enemy->next;
                        }
                        else
                        {
                            prev->next = enemy->next;
                        }
                        free(enemy);
                    }
                }
            }
        }
        else if (strcmp(cmd_buffer, "q") == 0)
        {
            break;
        }
        else
        {
            printf("Unknown command\n");
        }
    }
}