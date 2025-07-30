#include "dungeon.h"
#include <string.h>
#include <stdio.h>

int main()
{
    struct Dungeon *dungeon = load_dungeon("example.txt");
    int current_room = -1;
    for (int i = 0; i < dungeon->num_rooms; i++)
    {
        struct Room *room = &dungeon->rooms[i];
        for (int t = 0; t < room->num_tags; t++)
        {
            if (strcmp(room->tags[t], "START") == 0)
            {
                current_room = i;
                break;
            }
        }
        if (current_room != -1)
            break;
    }
    printf("\nStarting in %s: %s", dungeon->rooms[current_room].name, dungeon->rooms[current_room].desc);
}
