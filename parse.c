#include "dungeon.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct SpellEffectTmp
{
    struct SpellEffectTmp *next;
    int amount;
    enum SpellEffectType type;
};

struct SpellTargetTmp
{
    struct SpellEffectTmp *effects;
    struct SpellTargetTmp *next;
    int num_effects;
    enum SpellTargetType type;
};

struct SpellTmp
{
    char name[32];
    struct TagTmp *tags;
    struct SpellTargetTmp *targets;
    struct SpellTmp *next;
    int num_tags;
    int num_targets;
    int cost;
};

struct ExitTmp
{
    char exit_to[32];
    struct ExitTmp *next;
    enum Direction exit_dir;
};

struct EnemyTmp
{
    char name[32];
    int hp;
    int atk;
    int def;
    int mana;
    struct EnemyTmp *next;
};

struct ItemTmp
{
    char name[32];
    struct ItemTmp *next;
    int atk;
    int def;
    int mana;
    char *grants;
    enum ItemType type;
};

struct TagTmp
{
    char tag[16];
    struct TagTmp *next;
};

struct RoomTmp
{
    char name[32];
    char desc[128];
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
    struct SpellTmp *spells;
    int num_rooms;
    int num_spells;
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
    struct DungeonTmp dungeon_tmp =
        {
            .num_rooms = 0,
            .rooms = 0,
            .num_spells = 0,
            .spells = 0,
        };
    char line_buffer[128];
    FILE *f = fopen(filename, "r");
    struct EnemyTmp *current_enemy = 0;
    struct ItemTmp *current_item = 0;
    struct SpellTmp *current_spell = 0;
    struct SpellTargetTmp *current_spell_target = 0;
    struct RoomTmp *current_room = 0;

    while (fgets(line_buffer, 128, f))
    {
        for (int i = strlen(line_buffer) - 1; i > 0 && isspace(line_buffer[i]); line_buffer[i--] = '\0')
            ;
        char *line = trim_wspace(line_buffer);
        if (strnicmp(line, "ROOM ", 5) == 0)
        {
            // starting a new room
            current_enemy = 0;
            current_item = 0;
            current_spell = 0;
            current_spell_target = 0;
            dungeon_tmp.num_rooms++;
            // allocate space for the new room
            current_room = malloc(sizeof(struct RoomTmp));
            // hook it up to the linked list
            current_room->next = dungeon_tmp.rooms;
            dungeon_tmp.rooms = current_room;
            // initialize all the other fields
            current_room->num_exits = 0;
            current_room->num_tags = 0;
            current_room->enemies = 0;
            current_room->exits = 0;
            current_room->items = 0;
            current_room->tags = 0;
            // copy the name over
            strncpy(current_room->name, trim_wspace(line + 5), 32);
            // set an empty description
            current_room->desc[0] = 0;
        }
        else if (strnicmp(line, "EXIT ", 5) == 0)
        {
            line = trim_wspace(line + 5);
            if (current_room == 0)
            {
                printf("ERROR: Attempt to add an exit without a room: `%s`\n", line);
            }
            current_enemy = 0;
            current_item = 0;
            enum Direction dir;
            char *exit;
            if (strnicmp(line, "NORTH ", 6) == 0)
            {
                dir = DIR_NORTH;
                exit = trim_wspace(line + 6);
            }
            else if (strnicmp(line, "SOUTH ", 6) == 0)
            {
                dir = DIR_SOUTH;
                exit = trim_wspace(line + 6);
            }
            else if (strnicmp(line, "EAST ", 5) == 0)
            {
                dir = DIR_EAST;
                exit = trim_wspace(line + 5);
            }
            else if (strnicmp(line, "WEST ", 5) == 0)
            {
                dir = DIR_WEST;
                exit = trim_wspace(line + 5);
            }
            else
            {
                printf("ERROR: Invalid direction: `%s`\n", line);
                continue;
            }
            struct ExitTmp *exit_tmp = malloc(sizeof(struct ExitTmp));
            exit_tmp->exit_dir = dir;
            strncpy(exit_tmp->exit_to, exit, 32);
            exit_tmp->next = current_room->exits;
            current_room->exits = exit_tmp;
            current_room->num_exits++;
        }
        else if (strnicmp(line, "ITEM ", 5) == 0)
        {
            current_enemy = 0;
            current_item = malloc(sizeof(struct ItemTmp));
            // copy the item name
            strncpy(current_item->name, trim_wspace(line + 5), 32);
            // hook up to the linked list
            current_item->next = current_room->items;
            current_item->atk = 0;
            current_item->def = 0;
            current_item->mana = 0;
            current_item->grants = 0;
            current_item->type = IT_DEFAULT;
            current_room->items = current_item;
        }
        else if (strnicmp(line, "GRANTS ", 7) == 0)
        {
            line = trim_wspace(line + 7);
            if (current_item == 0)
            {
                printf("ERROR: Attempt to use GRANTS without an item.\n");
                continue;
            }
            if (current_item->grants != 0)
            {
                printf("ERROR: An item may not grant more than one spell.\n");
            }
            current_item->grants = strdup(line);
        }
        else if (strnicmp(line, "EQUIP ", 6) == 0)
        {
            line = trim_wspace(line + 6);
            if (current_item == 0)
            {
                printf("ERROR: Attempt to use EQUIP without an item.\n");
                continue;
            }
            if (stricmp(line, "HEAD") == 0)
                current_item->type = IT_ARMOR_HEAD;
            else if (stricmp(line, "CHEST") == 0)
                current_item->type = IT_ARMOR_CHEST;
            else if (stricmp(line, "LEGS") == 0)
                current_item->type = IT_ARMOR_LEGS;
            else if (stricmp(line, "FEET") == 0)
                current_item->type = IT_ARMOR_FEET;
            else if (stricmp(line, "WEAPON") == 0)
                current_item->type = IT_WEAPON;
            else if (stricmp(line, "SHIELD") == 0)
                current_item->type = IT_SHIELD;
            else
            {
                printf("ERROR: Invalid equip type: `%s`\n", line);
                continue;
            }
        }
        else if (strnicmp(line, "ENEMY ", 6) == 0)
        {
            line = trim_wspace(line + 6);
            if (current_room == 0)
            {
                printf("ERROR: Attempt to add enemy without room: `%s`", line);
                continue;
            }
            current_item = 0;
            current_enemy =
                malloc(sizeof(struct EnemyTmp));
            // copy the enemy name
            strncpy(current_enemy->name, line, 32);
            // hook up to the linked list
            current_enemy->next = current_room->enemies;
            current_room->enemies = current_enemy;
            current_enemy->hp = 1;
            current_enemy->atk = 1;
            current_enemy->def = 0;
            current_enemy->mana = 0;
        }
        else if (strnicmp(line, "HP ", 3) == 0)
        {
            int hp = -1;
            line = trim_wspace(line + 3);
            sscanf(line, "%u", &hp);
            if (hp == -1)
            {
                printf("ERROR: Could not parse HP: `%s`\n", line);
                continue;
            }
            if (current_enemy == 0)
            {
                printf("ERROR: Attempt to add HP without enemy.\n");
                continue;
            }
            else
                current_enemy->hp = hp;
        }
        else if (strnicmp(line, "MANA ", 4) == 0)
        {
            int mana = -1;
            line = trim_wspace(line + 4);
            sscanf(line, "%u", &mana);
            if (mana == -1)
            {
                printf("ERROR: Could not parse MANA: `%s`\n", line);
                continue;
            }
            if (current_enemy != 0)
                current_enemy->mana = mana;
            else if (current_item != 0)
                current_item->mana = mana;
            else
                printf("ERROR: Attempt to add MANA without item or enemy.\n");
        }
        else if (strnicmp(line, "ATK ", 4) == 0)
        {
            int atk = -1;
            line = trim_wspace(line + 4);
            sscanf(line, "%u", &atk);
            if (atk == -1)
            {
                printf("ERROR: Could not parse ATK: `%s`\n", line);
                continue;
            }
            if (current_item != 0)
                current_item->atk = atk;
            else if (current_enemy != 0)
                current_enemy->atk = atk;
            else
                printf("ERROR: Attempt to add ATK without item or enemy.\n");
        }
        else if (strnicmp(line, "DEF ", 4) == 0)
        {
            int def = -1;
            line = trim_wspace(line + 4);
            sscanf(line, "%u", &def);
            if (def == -1)
            {
                printf("ERROR: Could not parse DEF: `%s`\n", line);
                continue;
            }
            if (current_item != 0)
                current_item->def = def;
            else if (current_enemy != 0)
                current_enemy->def = def;
            else
                printf("ERROR: Attempt to add DEF without item or enemy.\n");
        }
        else if (strnicmp(line, "DESC ", 5) == 0)
        {
            if (current_room == 0)
            {
                printf("ERROR: Attempt to add a description without a room.\n");
                continue;
            }
            // add a description to the current room
            strncpy(current_room->desc, trim_wspace(line + 5), 128);
        }
        else if (strnicmp(line, "TAG ", 4) == 0)
        {
            struct TagTmp *tag_tmp = malloc(sizeof(struct TagTmp));
            // copy the tag name
            strncpy(tag_tmp->tag, trim_wspace(line + 4), 16);
            // hook up to the linked list
            if (current_room != 0)
            {
                tag_tmp->next = current_room->tags;
                current_room->tags = tag_tmp;
                current_room->num_tags++;
            }
            else if (current_spell != 0)
            {
                tag_tmp->next = current_spell->tags;
                current_spell->tags = tag_tmp;
                current_spell->num_tags++;
            }
            else
            {
                printf("ERROR: Attempt to add tag without a spell or room: `%s`\n", line + 4);
                continue;
            }
        }
        else if (strnicmp(line, "SPELL ", 6) == 0)
        {
            current_enemy = 0;
            current_item = 0;
            current_spell_target = 0;
            current_room = 0;
            line = trim_wspace(line + 6);
            current_spell = malloc(sizeof(struct SpellTmp));
            strncpy(current_spell->name, line, 32);
            // set all the default values
            current_spell->num_tags = 0;
            current_spell->num_targets = 0;
            current_spell->targets = 0;
            current_spell->tags = 0;
            // hook into the linked list
            current_spell->next = dungeon_tmp.spells;
            dungeon_tmp.spells = current_spell;
            dungeon_tmp.num_spells++;
        }
        else if (strnicmp(line, "COST ", 5) == 0)
        {
            int cost = -1;
            line = trim_wspace(line + 5);
            sscanf(line, "%u", &cost);
            if (cost == -1)
            {
                printf("ERROR: Could not parse cost: `%s`\n", line);
                continue;
            }
            if (current_spell == 0)
            {
                printf("ERROR: Attempt to add cost without spell.\n");
                continue;
            }
            else
                current_spell->cost = cost;
        }
        else if (strncmp(line, "TARGET ", 7) == 0)
        {
            enum SpellTargetType type;
            line = trim_wspace(line + 7);
            if (strcmp(line, "ENEMY") == 0)
                type = ST_TARGET_ENEMY;
            else if (strcmp(line, "ALLY") == 0)
                type = ST_TARGET_ALLY;
            else if (strcmp(line, "SELF") == 0)
                type = ST_SELF;
            else
            {
                printf("ERROR: Invalid TARGET type: `%s`; expected `ENEMY`, `ALLY`, or `SELF`\n", line);
                continue;
            }
            current_spell_target = malloc(sizeof(struct SpellTargetTmp));
            // set default values
            current_spell_target->effects = 0;
            current_spell_target->num_effects = 0;
            current_spell_target->type = type;
            // hook into the linked list
            current_spell_target->next = current_spell->targets;
            current_spell->targets = current_spell_target;
            current_spell->num_targets++;
        }
        else if (strncmp(line, "EACH ", 5) == 0)
        {
            enum SpellTargetType type;
            line = trim_wspace(line + 5);
            if (strcmp(line, "ENEMY") == 0)
                type = ST_EACH_ENEMY;
            else if (strcmp(line, "ALLY") == 0)
                type = ST_EACH_ALLY;
            else
            {
                printf("ERROR: Invalid EACH type: `%s`; expected `ENEMY` or `ALLY`\n", line);
                continue;
            }
            current_spell_target = malloc(sizeof(struct SpellTargetTmp));
            // set default values
            current_spell_target->effects = 0;
            current_spell_target->num_effects = 0;
            current_spell_target->type = type;
            // hook into the linked list
            current_spell_target->next = current_spell->targets;
            current_spell->targets = current_spell_target;
            current_spell->num_targets++;
        }
        else if (strncmp(line, "EFFECT ", 7) == 0)
        {
            char effect[8];
            int amount;
            line = trim_wspace(line + 7);
            if (sscanf(line, "%s %i", effect, &amount) <= 0)
            {
                printf("ERROR: Invalid EFFECT statement: `%s`. Usage is EFFECT <name> <amount>\n", line);
                continue;
            }
            enum SpellEffectType type;
            if (strcmp(effect, "RAGE") == 0)
                type = SE_RAGE;
            else if (strcmp(effect, "FORTIFY") == 0)
                type = SE_FORTIFY;
            else if (strcmp(effect, "WEAK") == 0)
                type = SE_WEAK;
            else if (strcmp(effect, "BURN") == 0)
                type = SE_BURN;
            else if (strcmp(effect, "POISON") == 0)
                type = SE_POISON;
            else if (strcmp(effect, "STUN") == 0)
                type = SE_STUN;
            else if (strcmp(effect, "REGEN") == 0)
                type = SE_REGEN;
            else
            {
                printf("ERROR: Invalid EFFECT type: `%s`\n", line);
            }
            struct SpellEffectTmp *current_effect = malloc(sizeof(struct SpellEffectTmp));
            // set our values
            current_effect->amount = amount;
            current_effect->type = type;
            // hook into the linked list
            current_spell_target->num_effects++;
            current_effect->next = current_spell_target->effects;
            current_spell_target->effects = current_effect;
        }
        else if (strncmp(line, "DMG ", 4) == 0)
        {
            int amount;
            if (sscanf(line + 4, "%i", &amount) <= 0)
            {
                printf("ERROR: Invalid DMG value: `%s`; expected a number\n", line + 5);
                continue;
            }
            struct SpellEffectTmp *current_effect = malloc(sizeof(struct SpellEffectTmp));
            // set our values
            current_effect->amount = amount;
            current_effect->type = SE_DMG;
            // hook into the linked list
            current_spell_target->num_effects++;
            current_effect->next = current_spell_target->effects;
            current_spell_target->effects = current_effect;
        }
        else if (strncmp(line, "HEAL ", 5) == 0)
        {
            int amount;
            if (sscanf(line + 5, "%i", &amount) <= 0)
            {
                printf("ERROR: Invalid HEAL value: `%s`; expected a number\n", line + 5);
                continue;
            }
            struct SpellEffectTmp *current_effect = malloc(sizeof(struct SpellEffectTmp));
            // set our values
            current_effect->amount = amount;
            current_effect->type = SE_HEAL;
            // hook into the linked list
            current_spell_target->num_effects++;
            current_effect->next = current_spell_target->effects;
            current_spell_target->effects = current_effect;
        }
        else if (*line != 0)
        {
            printf("ERROR: Could not parse: `%s`\n", line);
        }
        // ignore empty/invalid lines (?)
    }
    fclose(f);
    // print_dungeon_tmp(dungeon_tmp);
    // allocate the dungeon with enough space to store all the rooms
    struct Dungeon *dungeon = malloc(sizeof(struct Dungeon) + sizeof(struct Room) * dungeon_tmp.num_rooms);

    dungeon->num_spells = dungeon_tmp.num_spells;
    dungeon->spells = malloc(sizeof(struct Spell) * dungeon_tmp.num_spells);
    // loop through the spells of the tmp and final dungeon in parallel
    struct Spell *spell = dungeon->spells;
    for (struct SpellTmp *spell_tmp = dungeon_tmp.spells; spell_tmp != 0; spell_tmp = spell_tmp->next)
    {
        // move over the trivial fields (cost+name)
        spell->cost = spell_tmp->cost;
        strcpy(spell->name, spell_tmp->name);
        // move over the target blocks
        spell->num_targets = spell_tmp->num_targets;
        spell->targets = malloc(sizeof(struct SpellTarget) * spell->num_targets);
        struct SpellTarget *target = spell->targets;
        for (struct SpellTargetTmp *target_tmp = spell_tmp->targets; target_tmp != 0; target_tmp = target_tmp->next)
        {
            // copy over the target type
            target->type = target_tmp->type;
            // move over the effects
            target->num_effects = target_tmp->num_effects;
            target->effects = malloc(sizeof(struct SpellEffect) * target_tmp->num_effects);
            struct SpellEffect *effect = target->effects;
            for (struct SpellEffectTmp *effect_tmp = target_tmp->effects; effect_tmp != 0; effect_tmp = effect_tmp->next)
            {
                // copy over the fields
                effect->amount = effect_tmp->amount;
                effect->type = effect_tmp->type;
                // go to the next effect in the array
                effect++;
            }
            // go to the next target block in the array
            target++;
        }
        // move over the tags
        spell->num_tags = spell_tmp->num_tags;
        spell->tags = malloc(sizeof(char *) * spell->num_tags);
        char **tag = spell->tags;
        for (struct TagTmp *tag_tmp = spell_tmp->tags; tag_tmp != 0; tag_tmp = tag_tmp->next)
        {
            *tag = strdup(tag_tmp->tag);
            tag++;
        }
        // go to the next spell in the array
        spell++;
    }

    dungeon->num_rooms = dungeon_tmp.num_rooms;
    // loop through the rooms of the tmp and final dungeon in parallel
    struct Room *room = dungeon->rooms;
    for (struct RoomTmp *room_tmp = dungeon_tmp.rooms; room_tmp != 0; room_tmp = room_tmp->next)
    {
        // start with an empty list
        room->enemies = 0;
        room->items = 0;
        // move over the name and description
        strncpy(room->name, room_tmp->name, 32);
        strncpy(room->desc, room_tmp->desc, 128);
        // move over the enemies
        for (struct EnemyTmp *enemy_tmp = room_tmp->enemies; enemy_tmp != 0; enemy_tmp = enemy_tmp->next)
        {
            struct Enemy *enemy = malloc(sizeof(struct Enemy));
            strncpy(enemy->stats.name, enemy_tmp->name, 32);
            enemy->stats.burn = 0;
            enemy->stats.fortify = 0;
            enemy->stats.poison = 0;
            enemy->stats.rage = 0;
            enemy->stats.regen = 0;
            enemy->stats.stun = 0;
            enemy->stats.hp = enemy_tmp->hp;
            enemy->stats.atk = enemy_tmp->atk;
            enemy->stats.def = enemy_tmp->def;
            enemy->stats.mana = enemy_tmp->mana;
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
                if (stricmp(exit_tmp->exit_to, room_candidate->name) == 0)
                {
                    exit->room = i;
                    break;
                }
                i++;
            }
            if (exit->room == -1)
                printf("ERROR: Can't find room %s (linking exits for %s)\n", exit_tmp->exit_to, room_tmp->name);
            exit++;
        }
        // move over the items
        for (struct ItemTmp *item_tmp = room_tmp->items; item_tmp != 0; item_tmp = item_tmp->next)
        {
            struct Item *item = malloc(sizeof(struct Item));
            strncpy(item->name, item_tmp->name, 32);
            item->atk = item_tmp->atk;
            item->def = item_tmp->def;
            item->mana = item_tmp->mana;
            item->type = item_tmp->type;
            item->next = room->items;
            item->grants = 0;
            if (item_tmp->grants != 0)
            {
                int i = 0;
                for (struct SpellTmp *spell = dungeon_tmp.spells; spell != 0; spell = spell->next)
                {
                    if (strcmp(spell->name, item_tmp->grants) == 0)
                    {
                        item->grants = dungeon->spells + i;
                        break;
                    }
                    i++;
                }
                if (item->grants == 0)
                {
                    printf("ERROR: Failed to find spell `%s`, granted by item `%s`\n", item_tmp->grants, item_tmp->name);
                }
            }
            room->items = item;
        }
        // move over the tags
        room->num_tags = room_tmp->num_tags;
        room->tags = malloc(sizeof(char *) * room->num_tags);
        char **tag = room->tags;
        for (struct TagTmp *tag_tmp = room_tmp->tags; tag_tmp != 0; tag_tmp = tag_tmp->next)
        {
            *tag = strdup(tag_tmp->tag);
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
    for (struct SpellTmp *spell = dungeon_tmp.spells; spell != 0;)
    {
        for (struct SpellTargetTmp *target = spell->targets; target != 0;)
        {
            for (struct SpellEffectTmp *effect = target->effects; effect != 0;)
            {
                struct SpellEffectTmp *e = effect->next;
                free(effect);
                effect = e;
            }
            struct SpellTargetTmp *t = target->next;
            free(target);
            target = t;
        }
        for (struct TagTmp *tag = spell->tags; tag != 0;)
        {
            struct TagTmp *nxt = tag->next;
            free(tag);
            tag = nxt;
        }
        struct SpellTmp *s = spell->next;
        free(spell);
        spell = s;
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
            printf(" DESC %s\n", room->desc);
        for (struct TagTmp *tag = room->tags; tag != 0; tag = tag->next)
            printf(" TAG %s\n", tag->tag);
        for (struct EnemyTmp *enemy = room->enemies; enemy != 0; enemy = enemy->next)
            printf(" ENEMY %s\n  HP %u\n  ATK %u\n  DEF %u\n", enemy->name, enemy->hp, enemy->atk, enemy->def);
        for (struct ItemTmp *item = room->items; item != 0; item = item->next)
            printf(" ITEM %s\n", item->name);
        for (struct ExitTmp *exit = room->exits; exit != 0; exit = exit->next)
            printf(" EXIT %s %s\n", fmt_dir(exit->exit_dir), exit->exit_to);
    }
}

void print_dungeon(struct Dungeon *dungeon)
{
    for (int room_id = 0; room_id < dungeon->num_rooms; room_id++)
    {
        struct Room *room = &dungeon->rooms[room_id];
        printf("\nROOM %s\n", room->name);
        if (*room->desc)
            printf(" DESC %s\n", room->desc);
        for (int tag_id = 0; tag_id < room->num_tags; tag_id++)
            printf(" TAG %s\n", room->tags[tag_id]);
        for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
            printf(" ENEMY %s\n\n  HP %u\n  ATK %u\n  DEF %u\n", enemy->stats.name, enemy->stats.hp, enemy->stats.atk, enemy->stats.def);
        for (struct Item *item = room->items; item != 0; item = item->next)
            printf(" ITEM %s\n", item->name);
        for (int exit_id = 0; exit_id < room->num_exits; exit_id++)
        {
            struct Exit *exit = &room->exits[exit_id];
            printf(" EXIT %s %s\n", fmt_dir(exit->dir), dungeon->rooms[exit->room].name);
        }
    }
}