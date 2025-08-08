#include "dungeon.h"
#ifndef _PARSE_H
#define _PARSE_H

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
    char grants[32];
    struct ItemTmp *next;
    struct AbilityTmp *abilities;
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

struct NpcInstructionTmp
{
    enum NpcOpcode opcode;
    union
    {
        char text[32];
        int id;
        struct EnemyTmp *enemy;
        struct ItemTmp *item;
    } data;
    struct NpcInstructionTmp *next;
};

struct NpcOptionTmp
{
    char text[32];
    struct NpcInstructionTmp *instructions;
    struct NpcOptionTmp *next;
    int num_instructions;
};

struct NpcStateTmp
{
    char name[32];
    struct NpcInstructionTmp *instructions;
    struct NpcOptionTmp *options;
    struct NpcStateTmp *next;
    int num_instructions;
    int num_options;
};

struct NpcTmp
{
    char name[32];
    struct NpcTmp *next;
    struct NpcStateTmp *states;
    struct TagTmp *flags;
    int num_states;
    int num_flags;
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
    struct NpcTmp *npcs;
};

struct DungeonTmp
{
    struct RoomTmp *rooms;
    struct SpellTmp *spells;
    struct ItemTmp *item_templates;
    struct EnemyTmp *enemy_templates;
    int num_rooms;
    int num_spells;
};

void print_dungeon_tmp(struct DungeonTmp);
void print_dungeon(struct Dungeon *);
struct Enemy *convert_enemy(struct EnemyTmp *enemy_tmp, struct Dungeon *dungeon, struct DungeonTmp *dungeon_tmp);

#endif
