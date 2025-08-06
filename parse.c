#include "dungeon.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct SpellStatusTmp
{
    struct SpellStatusTmp *next;
    int amount;
    enum SpellEffectType type;
};

union SpellEffectTmp
{
    struct
    {
        struct SpellStatusTmp *effects;
        int num_effects;
    } status;
    struct EnemyTmp *enemy;
};

struct SpellBlockTmp
{
    union SpellEffectTmp effects;
    struct SpellBlockTmp *next;
    enum SpellBlockType type;
};

struct SpellTmp
{
    char name[32];
    struct TagTmp *tags;
    struct SpellBlockTmp *blocks;
    struct SpellTmp *next;
    int num_tags;
    int num_blocks;
    int cost;
};

struct ExitTmp
{
    char exit_to[32];
    char *key;
    struct ExitTmp *next;
    enum Direction exit_dir;
};

struct AbilityTmp
{
    char result[32];
    enum Trigger trigger;
    struct AbilityTmp *next;
};

struct EnemyTmp
{
    char name[32];
    struct EnemyTmp *next;
    struct ItemTmp *drops;
    struct AbilityTmp *abilities;
    int hp;
    int atk;
    int def;
    int mana;
    int num_abilities;
};

struct ItemTmp
{
    char name[32];
    struct ItemTmp *next;
    struct AbilityTmp *abilities;
    char *grants;
    int atk;
    int def;
    int mana;
    int num_abilities;
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
struct Enemy *copy_enemy(struct EnemyTmp *enemy_tmp, struct Dungeon *dungeon, struct DungeonTmp *dungeon_tmp);

char *trim_wspace(char *p)
{
    while (isspace(*p++))
        ;
    return --p;
}

struct RoomTmp *new_room(char *name)
{
    // allocate space for the new room
    struct RoomTmp *room = malloc(sizeof(struct RoomTmp));
    // initialize all the other fields
    room->num_exits = 0;
    room->num_tags = 0;
    room->enemies = 0;
    room->exits = 0;
    room->items = 0;
    room->tags = 0;
    // copy the name over
    strncpy(room->name, name, 32);
    // set an empty description
    room->desc[0] = 0;
    return room;
}

struct ItemTmp *new_item(char *name)
{
    struct ItemTmp *i = malloc(sizeof(struct ItemTmp));
    // copy the item name
    strncpy(i->name, name, 32);
    // add the default values
    i->atk = 0;
    i->def = 0;
    i->mana = 0;
    i->grants = 0;
    i->type = IT_DEFAULT;
    i->abilities = 0;
    i->num_abilities = 0;
    return i;
}

struct EnemyTmp *new_enemy(char *name)
{
    struct EnemyTmp *e = malloc(sizeof(struct EnemyTmp));
    // copy the enemy name
    strncpy(e->name, name, 32);
    // initialize default values
    e->hp = 1;
    e->atk = 1;
    e->def = 0;
    e->mana = 0;
    e->drops = 0;
    e->abilities = 0;
    e->num_abilities = 0;
    return e;
}

void copy_spell(struct Spell *spell, struct SpellTmp *spell_tmp, struct Dungeon *dungeon, struct DungeonTmp *dungeon_tmp)
{
    // move over the trivial fields (cost+name)
    spell->cost = spell_tmp->cost;
    strcpy(spell->name, spell_tmp->name);
    // move over the target blocks
    spell->num_blocks = spell_tmp->num_blocks;
    spell->blocks = malloc(sizeof(struct SpellBlock) * spell->num_blocks);
    struct SpellBlock *target = spell->blocks;
    for (struct SpellBlockTmp *target_tmp = spell_tmp->blocks; target_tmp != 0; target_tmp = target_tmp->next)
    {
        // copy over the target type
        target->type = target_tmp->type;
        if (target->type == ST_SUMMON)
        {
            // move over the entity
            target->effect.summon = copy_enemy(target_tmp->effects.enemy, dungeon, dungeon_tmp);
        }
        else
        {
            // move over the effects
            target->effect.status.num_effects = target_tmp->effects.status.num_effects;
            target->effect.status.effects = malloc(sizeof(struct SpellStatus) * target_tmp->effects.status.num_effects);
            struct SpellStatus *effect = target->effect.status.effects;
            for (struct SpellStatusTmp *effect_tmp = target_tmp->effects.status.effects; effect_tmp != 0; effect_tmp = effect_tmp->next)
            {
                // copy over the fields
                effect->amount = effect_tmp->amount;
                effect->type = effect_tmp->type;
                // go to the next effect in the array
                effect++;
            }
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
}

struct Spell *find_spell(char *name, struct Dungeon *dungeon, struct DungeonTmp *dungeon_tmp)
{
    int i = 0;
    for (struct SpellTmp *spell = dungeon_tmp->spells; spell != 0; spell = spell->next, i++)
        if (strcmp(spell->name, name) == 0)
            return dungeon->spells + i;
    return 0;
}

struct Ability *copy_abilities(int amount, struct AbilityTmp *ability, struct Dungeon *dungeon, struct DungeonTmp *dungeon_tmp)
{
    struct Ability *a = malloc(sizeof(struct Ability) * amount);
    for (struct Ability *a_t = a; ability != 0; ability = ability->next, a_t++)
    {
        a_t->trigger = ability->trigger;
        a_t->result = find_spell(ability->result, dungeon, dungeon_tmp);
        if (a_t->result == 0)
            printf("ERROR: Failed to find spell `%s`\n", ability->result);
    }
    return a;
}

struct Item *copy_item(struct ItemTmp *item_tmp, struct Dungeon *dungeon, struct DungeonTmp *dungeon_tmp)
{
    struct Item *item = malloc(sizeof(struct Item));
    strncpy(item->name, item_tmp->name, 32);
    item->atk = item_tmp->atk;
    item->def = item_tmp->def;
    item->mana = item_tmp->mana;
    item->type = item_tmp->type;
    item->grants = 0;
    if (item_tmp->grants != 0)
    {
        item->grants = find_spell(item_tmp->grants, dungeon, dungeon_tmp);
        if (item->grants == 0)
            printf("ERROR: Failed to find spell `%s`, granted by item `%s`\n", item_tmp->grants, item_tmp->name);
    }
    item->num_abilities = item_tmp->num_abilities;
    item->abilities = copy_abilities(item->num_abilities, item_tmp->abilities, dungeon, dungeon_tmp);
    return item;
}

struct Enemy *copy_enemy(struct EnemyTmp *enemy_tmp, struct Dungeon *dungeon, struct DungeonTmp *dungeon_tmp)
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
    enemy->stats.max_hp = enemy_tmp->hp;
    enemy->stats.atk = enemy_tmp->atk;
    enemy->stats.def = enemy_tmp->def;
    enemy->stats.mana = enemy_tmp->mana;
    enemy->num_abilities = enemy_tmp->num_abilities;
    enemy->abilities = copy_abilities(enemy->num_abilities, enemy_tmp->abilities, dungeon, dungeon_tmp);
    enemy->drops = 0;
    for (struct ItemTmp *item_tmp = enemy_tmp->drops; item_tmp != 0; item_tmp = item_tmp->next)
    {
        struct Item *item = copy_item(item_tmp, dungeon, dungeon_tmp);
        // hook into the linked list
        item->next = enemy->drops;
        enemy->drops = item;
    }
    return enemy;
}

void copy_room(struct Room *room, struct RoomTmp *room_tmp, struct Dungeon *dungeon, struct DungeonTmp *dungeon_tmp)
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
        struct Enemy *enemy = copy_enemy(enemy_tmp, dungeon, dungeon_tmp);
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
        exit->key = exit_tmp->key;
        // default room in case something goes wrong
        exit->room = 0;
        // find the ID of the room the exit is going to
        int i = 0;
        for (struct RoomTmp *room_candidate = dungeon_tmp->rooms; room_candidate != 0; room_candidate = room_candidate->next)
        {
            if (strcmp(exit_tmp->exit_to, room_candidate->name) == 0)
            {
                exit->room = &dungeon->rooms[i];
                break;
            }
            i++;
        }
        if (exit->room == 0)
            printf("ERROR: Can't find room %s (linking exits for %s)\n", exit_tmp->exit_to, room_tmp->name);
        exit++;
    }
    // move over the items
    for (struct ItemTmp *item_tmp = room_tmp->items; item_tmp != 0; item_tmp = item_tmp->next)
    {
        struct Item *item = copy_item(item_tmp, dungeon, dungeon_tmp);
        item->next = room->items;
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
}

void free_item(struct ItemTmp *item)
{
    for (struct AbilityTmp *ability = item->abilities; ability != 0;)
    {
        struct AbilityTmp *nxt_a = ability->next;
        free(ability);
        ability = nxt_a;
    }
    free(item);
}

void free_enemy(struct EnemyTmp *enemy)
{
    for (struct AbilityTmp *ability = enemy->abilities; ability != 0;)
    {
        struct AbilityTmp *nxt_a = ability->next;
        free(ability);
        ability = nxt_a;
    }
    for (struct ItemTmp *item = enemy->drops; item != 0;)
    {
        struct ItemTmp *nxt_i = item->next;
        free_item(item);
        item = nxt_i;
    }
    free(enemy);
}

void free_dungeon(struct DungeonTmp *dungeon_tmp)
{

    for (struct RoomTmp *room = dungeon_tmp->rooms; room != 0;)
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
            free_enemy(enemy);
            enemy = nxt;
        }
        for (struct ItemTmp *item = room->items; item != 0;)
        {
            struct ItemTmp *nxt = item->next;
            free_item(item);
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
    for (struct SpellTmp *spell = dungeon_tmp->spells; spell != 0;)
    {
        for (struct SpellBlockTmp *block = spell->blocks; block != 0;)
        {
            if (block->type == ST_SUMMON)
            {
                free_enemy(block->effects.enemy);
            }
            else
                for (struct SpellStatusTmp *effect = block->effects.status.effects; effect != 0;)
                {
                    struct SpellStatusTmp *e = effect->next;
                    free(effect);
                    effect = e;
                }
            struct SpellBlockTmp *t = block->next;
            free(block);
            block = t;
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
}

struct Dungeon *copy_dungeon(struct DungeonTmp *dungeon_tmp)
{
    // allocate the dungeon with enough space to store all the rooms
    struct Dungeon *dungeon = malloc(sizeof(struct Dungeon) + sizeof(struct Room) * dungeon_tmp->num_rooms);

    dungeon->num_spells = dungeon_tmp->num_spells;
    dungeon->spells = malloc(sizeof(struct Spell) * dungeon_tmp->num_spells);
    // loop through the spells of the tmp and final dungeon in parallel
    struct Spell *spell = dungeon->spells;
    for (struct SpellTmp *spell_tmp = dungeon_tmp->spells; spell_tmp != 0; spell_tmp = spell_tmp->next)
        copy_spell(spell++, spell_tmp, dungeon, dungeon_tmp);
    dungeon->num_rooms = dungeon_tmp->num_rooms;
    // loop through the rooms of the tmp and final dungeon in parallel
    struct Room *room = dungeon->rooms;
    for (struct RoomTmp *room_tmp = dungeon_tmp->rooms; room_tmp != 0; room_tmp = room_tmp->next)
        copy_room(room++, room_tmp, dungeon, dungeon_tmp);
    return dungeon;
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
    struct SpellBlockTmp *current_spell_target = 0;
    struct RoomTmp *current_room = 0;
    struct ExitTmp *current_exit = 0;

    while (fgets(line_buffer, 128, f))
    {
        for (int i = strlen(line_buffer) - 1; i > 0 && isspace(line_buffer[i]); line_buffer[i--] = '\0')
            ;
        char *line = trim_wspace(line_buffer);
        if (strncmp(line, "ROOM ", 5) == 0)
        {
            // starting a new room
            current_enemy = 0;
            current_item = 0;
            current_spell = 0;
            current_spell_target = 0;
            current_exit = 0;
            // new room
            current_room = new_room(trim_wspace(line + 5));
            // hook it up to the linked list
            current_room->next = dungeon_tmp.rooms;
            dungeon_tmp.rooms = current_room;
            dungeon_tmp.num_rooms++;
        }
        else if (strncmp(line, "EXIT ", 5) == 0)
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
            if (strncmp(line, "NORTH ", 6) == 0)
            {
                dir = DIR_NORTH;
                exit = trim_wspace(line + 6);
            }
            else if (strncmp(line, "SOUTH ", 6) == 0)
            {
                dir = DIR_SOUTH;
                exit = trim_wspace(line + 6);
            }
            else if (strncmp(line, "EAST ", 5) == 0)
            {
                dir = DIR_EAST;
                exit = trim_wspace(line + 5);
            }
            else if (strncmp(line, "WEST ", 5) == 0)
            {
                dir = DIR_WEST;
                exit = trim_wspace(line + 5);
            }
            else
            {
                printf("ERROR: Invalid direction: `%s`\n", line);
                continue;
            }
            current_exit = malloc(sizeof(struct ExitTmp));
            current_exit->exit_dir = dir;
            strncpy(current_exit->exit_to, exit, 32);
            current_exit->key = 0;
            // hook into the linked list
            current_exit->next = current_room->exits;
            current_room->exits = current_exit;
            current_room->num_exits++;
        }
        else if (strncmp(line, "KEY ", 4) == 0)
        {
            if (current_exit == 0)
            {
                printf("ERROR: Attempt to add KEY without an exit.\n");
                continue;
            }
            if (current_exit->key != 0)
            {
                printf("ERROR: An exit may only have one key.\n");
                continue;
            }
            current_exit->key = strdup(trim_wspace(line + 4));
        }
        else if (strncmp(line, "ITEM ", 5) == 0)
        {
            current_enemy = 0;
            current_exit = 0;
            current_item = new_item(trim_wspace(line + 5));
            // hook up to the linked list
            current_item->next = current_room->items;
            current_room->items = current_item;
        }
        else if (strncmp(line, "DROP ", 5) == 0)
        {
            line = trim_wspace(line + 5);
            if (current_enemy == 0)
            {
                printf("ERROR: Attempt to add dropped item `%s` without enemy.\n", line);
                continue;
            }
            current_item = new_item(line);
            // hook up to the linked list
            current_item->next = current_enemy->drops;
            current_enemy->drops = current_item;
        }
        else if (strncmp(line, "GRANTS ", 7) == 0)
        {
            line = trim_wspace(line + 7);
            if (current_item == 0)
            {
                printf("ERROR: Attempt to add GRANTS without an item.\n");
                continue;
            }
            if (current_item->grants != 0)
            {
                printf("ERROR: An item may not grant more than one spell.\n");
                continue;
            }
            current_item->grants = strdup(line);
        }
        else if (strncmp(line, "CONSUME ", 8) == 0)
        {
            line = trim_wspace(line + 8);
            if (current_item == 0)
            {
                printf("ERROR: Attempt to add CONSUME without item.\n");
                continue;
            }
            if (current_item->grants != 0)
            {
                printf("ERROR: An item may not grant more than one spell.\n");
                continue;
            }
            if (current_item->type != IT_DEFAULT)
            {
                printf("ERROR: An item may only be marked with one EQUIP or CONSUME.\n");
                continue;
            }
            current_item->grants = strdup(line);
            current_item->type = IT_CONSUME;
        }
        else if (strncmp(line, "EQUIP ", 6) == 0)
        {
            line = trim_wspace(line + 6);
            if (current_item == 0)
            {
                printf("ERROR: Attempt to add EQUIP without an item.\n");
                continue;
            }
            if (current_item->type != IT_DEFAULT)
            {
                printf("ERROR: An item may only be marked with one EQUIP or CONSUME.\n");
                continue;
            }
            if (strcmp(line, "HEAD") == 0)
                current_item->type = IT_ARMOR_HEAD;
            else if (strcmp(line, "CHEST") == 0)
                current_item->type = IT_ARMOR_CHEST;
            else if (strcmp(line, "LEGS") == 0)
                current_item->type = IT_ARMOR_LEGS;
            else if (strcmp(line, "FEET") == 0)
                current_item->type = IT_ARMOR_FEET;
            else if (strcmp(line, "WEAPON") == 0)
                current_item->type = IT_WEAPON;
            else if (strcmp(line, "SHIELD") == 0)
                current_item->type = IT_SHIELD;
            else
            {
                printf("ERROR: Invalid equip type: `%s`\n", line);
                continue;
            }
        }
        else if (strncmp(line, "ENEMY ", 6) == 0)
        {
            line = trim_wspace(line + 6);
            if (current_room == 0)
            {
                printf("ERROR: Attempt to add enemy without room: `%s`", line);
                continue;
            }
            // update parser state
            current_exit = 0;
            current_item = 0;
            current_enemy = new_enemy(line);
            //  hook up to the linked list
            current_enemy->next = current_room->enemies;
            current_room->enemies = current_enemy;
        }
        else if (strncmp(line, "HP ", 3) == 0)
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
        else if (strncmp(line, "MANA ", 4) == 0)
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
        else if (strncmp(line, "ATK ", 4) == 0)
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
        else if (strncmp(line, "DEF ", 4) == 0)
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
        else if (strncmp(line, "DESC ", 5) == 0)
        {
            if (current_room == 0)
            {
                printf("ERROR: Attempt to add a description without a room.\n");
                continue;
            }
            // add a description to the current room
            strncpy(current_room->desc, trim_wspace(line + 5), 128);
        }
        else if (strncmp(line, "TAG ", 4) == 0)
        {
            current_exit = 0;
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
        else if (strncmp(line, "SPELL ", 6) == 0)
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
            current_spell->num_blocks = 0;
            current_spell->blocks = 0;
            current_spell->tags = 0;
            // hook into the linked list
            current_spell->next = dungeon_tmp.spells;
            dungeon_tmp.spells = current_spell;
            dungeon_tmp.num_spells++;
        }
        else if (strncmp(line, "COST ", 5) == 0)
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
            current_spell->cost = cost;
        }
        else if (strncmp(line, "TARGET ", 7) == 0)
        {
            enum SpellBlockType type;
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
            current_spell_target = malloc(sizeof(struct SpellBlockTmp));
            // set default values
            current_spell_target->effects.status.effects = 0;
            current_spell_target->effects.status.num_effects = 0;
            current_spell_target->type = type;
            // hook into the linked list
            current_spell_target->next = current_spell->blocks;
            current_spell->blocks = current_spell_target;
            current_spell->num_blocks++;
        }
        else if (strncmp(line, "EACH ", 5) == 0)
        {
            enum SpellBlockType type;
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
            current_spell_target = malloc(sizeof(struct SpellBlockTmp));
            // set default values
            current_spell_target->effects.status.effects = 0;
            current_spell_target->effects.status.num_effects = 0;
            current_spell_target->type = type;
            // hook into the linked list
            current_spell_target->next = current_spell->blocks;
            current_spell->blocks = current_spell_target;
            current_spell->num_blocks++;
        }
        else if (strncmp(line, "SUMMON ", 7) == 0)
        {
            line = trim_wspace(line + 7);
            if (current_spell == 0)
            {
                printf("ERROR: Attempt to add `SUMMON %s` without a spell.\n", line);
                continue;
            }
            current_enemy = new_enemy(line);
            struct SpellBlockTmp *enemy_block = malloc(sizeof(struct SpellBlockTmp));
            // set default values
            enemy_block->effects.enemy = current_enemy;
            enemy_block->type = ST_SUMMON;
            // hook into the linked list
            enemy_block->next = current_spell->blocks;
            current_spell->blocks = enemy_block;
            current_spell->num_blocks++;
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
            struct SpellStatusTmp *current_effect = malloc(sizeof(struct SpellStatusTmp));
            // set our values
            current_effect->amount = amount;
            current_effect->type = type;
            // hook into the linked list
            current_spell_target->effects.status.num_effects++;
            current_effect->next = current_spell_target->effects.status.effects;
            current_spell_target->effects.status.effects = current_effect;
        }
        else if (strncmp(line, "DMG ", 4) == 0)
        {
            int amount;
            if (sscanf(line + 4, "%i", &amount) <= 0)
            {
                printf("ERROR: Invalid DMG value: `%s`; expected a number\n", line + 5);
                continue;
            }
            struct SpellStatusTmp *current_effect = malloc(sizeof(struct SpellStatusTmp));
            // set our values
            current_effect->amount = amount;
            current_effect->type = SE_DMG;
            // hook into the linked list
            current_spell_target->effects.status.num_effects++;
            current_effect->next = current_spell_target->effects.status.effects;
            current_spell_target->effects.status.effects = current_effect;
        }
        else if (strncmp(line, "HEAL ", 5) == 0)
        {
            int amount;
            if (sscanf(line + 5, "%i", &amount) <= 0)
            {
                printf("ERROR: Invalid HEAL value: `%s`; expected a number\n", line + 5);
                continue;
            }
            struct SpellStatusTmp *current_effect = malloc(sizeof(struct SpellStatusTmp));
            // set our values
            current_effect->amount = amount;
            current_effect->type = SE_HEAL;
            // hook into the linked list
            current_spell_target->effects.status.num_effects++;
            current_effect->next = current_spell_target->effects.status.effects;
            current_spell_target->effects.status.effects = current_effect;
        }
        else if (strncmp(line, "ON ", 3) == 0)
        {
            if (current_enemy == 0 && current_item == 0)
            {
                printf("ERROR: Attempt to add ability without item or enemy: `%s`\n", line);
                continue;
            }
            line = trim_wspace(line + 3);
            enum Trigger trigger;
            if (strncmp(line, "ATK ", 4) == 0)
            {
                trigger = T_ATK;
                line = trim_wspace(line + 4);
            }
            else if (strncmp(line, "DEF ", 4) == 0)
            {
                trigger = T_DEF;
                line = trim_wspace(line + 4);
            }
            else if (strncmp(line, "DEATH ", 6) == 0)
            {
                trigger = T_DEATH;
                line = trim_wspace(line + 6);
            }
            else if (strncmp(line, "TURN ", 5) == 0)
            {
                trigger = T_TURN;
                line = trim_wspace(line + 5);
            }
            else
            {
                printf("ERROR: Unknown trigger type: `ON %s`\n", line);
                continue;
            }
            struct AbilityTmp *ability = malloc(sizeof(struct AbilityTmp));
            ability->trigger = trigger;
            // copy the spell name over
            strncpy(ability->result, line, 32);
            if (current_item != 0)
            {
                // add the ability to the current item
                current_item->num_abilities++;
                ability->next = current_item->abilities;
                current_item->abilities = ability;
            }
            else
            {
                // add the ability to the current enemy
                current_enemy->num_abilities++;
                ability->next = current_enemy->abilities;
                current_enemy->abilities = ability;
            }
        }
        else if (*line != 0)
        {
            printf("ERROR: Could not parse: `%s`\n", line);
        }
    }
    fclose(f);
    struct Dungeon *d = copy_dungeon(&dungeon_tmp);
    free_dungeon(&dungeon_tmp);
    return d;
}
