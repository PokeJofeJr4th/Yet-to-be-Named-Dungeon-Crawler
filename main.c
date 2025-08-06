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

void confirm()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void init_player(struct Player *p)
{
    p->stats.atk = 1;
    p->stats.hp = 10;
    p->stats.max_hp = 10;
    p->stats.def = 0;
    p->stats.mana = 0;
    p->stats.burn = 0;
    p->stats.fortify = 0;
    p->stats.poison = 0;
    p->stats.rage = 0;
    p->stats.regen = 0;
    p->stats.stun = 0;
    strcpy(p->stats.name, "You");
    for (int i = 0; i < NUM_EQ_SLOTS; i++)
        p->equipment[i].name[0] = 0;
    p->inventory = 0;
    p->spellbook = 0;
}

void update_stats(struct Player *p)
{
    p->stats.atk = 1;
    p->stats.def = 0;
    p->stats.mana = 0;
    for (int i = 0; i < NUM_EQ_SLOTS; i++)
        if (p->equipment[i].name[0])
        {
            p->stats.atk += p->equipment[i].atk;
            p->stats.def += p->equipment[i].def;
            p->stats.mana += p->equipment[i].mana;
        }
}

char *fmt_item_type(enum ItemType it)
{
    switch (it)
    {
    case IT_ARMOR_HEAD:
        return "head";
    case IT_ARMOR_CHEST:
        return "chest";
    case IT_ARMOR_LEGS:
        return "legs";
    case IT_ARMOR_FEET:
        return "feet";
    case IT_WEAPON:
        return "weapon";
    case IT_SHIELD:
        return "shield";
    case IT_CONSUME:
        return "consume";
    default:
        return "?";
    }
}

void print_item(struct Item *i)
{
    printf("%s", i->name);
    if (i->type != IT_DEFAULT)
    {
        printf(" (%s:", fmt_item_type(i->type));
        if (i->atk != 0)
            printf(" %+i ATK.", i->atk);
        if (i->def != 0)
            printf(" %+i DEF.", i->def);
        if (i->mana != 0)
            printf(" %+i MANA.", i->mana);
        if (i->grants != 0)
            printf(" <%s>.", i->grants->name);
        for (int j = 0; j < i->num_abilities; j++)
            printf(" on %s: <%s>.", fmt_ability_trigger(i->abilities[j].trigger), i->abilities[j].result->name);
        printf(")");
    }
    printf("\n");
}

void print_room(struct Room *room, struct Dungeon *dungeon)
{
    printf("%s\n %s\n", room->name, room->desc);
    if (room->enemies != 0)
    {
        printf("Enemies:\n");
        for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
        {

            printf(" %s (%u/%u HP, %u ATK, %u DEF", enemy->stats.name, enemy->stats.hp, enemy->stats.max_hp, enemy->stats.atk, enemy->stats.def);
            for (int j = 0; j < enemy->num_abilities; j++)
                printf(", on %s: <%s>", fmt_ability_trigger(enemy->abilities[j].trigger), enemy->abilities[j].result->name);
            printf(")\n");
        }
    }
    if (room->items != 0)
    {
        printf("Items:\n");
        for (struct Item *item = room->items; item != 0; item = item->next)
        {
            printf(" ");
            print_item(item);
        }
    }
    if (room->num_exits != 0)
    {
        printf("Exits:\n");
        for (int i = 0; i < room->num_exits; i++)
            printf(" %s: %s\n", fmt_dir(room->exits[i].dir), room->exits[i].room->name);
    }
}

void print_spell(struct Spell *spell)
{
    printf("%s (cost: %i)\n", spell->name, spell->cost);
    if (spell->num_tags != 0)
    {
        printf(" Tags: %s", spell->tags[0]);
        for (int i = 1; i < spell->num_tags; i++)
        {
            printf(", %s", spell->tags[i]);
        }
        printf("\n");
    }
    for (int i = 0; i < spell->num_blocks; i++)
    {
        struct SpellBlock t = spell->blocks[i];
        switch (t.type)
        {
        case ST_SUMMON:
            printf(" Summon %s\n", t.effect.summon->stats.name);
            continue;
        case ST_EACH_ALLY:
            printf(" All Allies:\n");
            break;
        case ST_TARGET_ALLY:
            printf(" Target an Ally:\n");
            break;
        case ST_EACH_ENEMY:
            printf(" All Enemies:\n");
            break;
        case ST_TARGET_ENEMY:
            printf(" Target an Enemy:\n");
            break;
        case ST_SELF:
            printf(" Self:\n");
            break;
        default:
            break;
        }
        for (int j = 0; j < t.effect.status.num_effects; j++)
        {
            switch (t.effect.status.effects[j].type)
            {
            case SE_BURN:
                printf("  Apply burn %i\n", t.effect.status.effects[j].amount);
                break;
            case SE_FORTIFY:
                printf("  Apply fortify %i\n", t.effect.status.effects[j].amount);
                break;
            case SE_POISON:
                printf("  Apply poison %i\n", t.effect.status.effects[j].amount);
                break;
            case SE_RAGE:
                printf("  Apply rage %i\n", t.effect.status.effects[j].amount);
                break;
            case SE_REGEN:
                printf("  Apply regeneration %i\n", t.effect.status.effects[j].amount);
                break;
            case SE_STUN:
                printf("  Apply stunned %i\n", t.effect.status.effects[j].amount);
                break;
            case SE_WEAK:
                printf("  Apply weakness %i\n", t.effect.status.effects[j].amount);
                break;
            case SE_DMG:
                printf("  Deal %i damage\n", t.effect.status.effects[j].amount);
                break;
            case SE_HEAL:
                printf("  Heal %i\n", t.effect.status.effects[j].amount);
                break;
            default:
                break;
            }
        }
    }
}

int main(int argc, char **argv)
{
    struct Dungeon *dungeon;
    if (argc == 1)
        dungeon = load_dungeon("example.txt");
    else
        dungeon = load_dungeon(argv[1]);
    struct Room *room = 0;
    struct Player player;
    init_player(&player);
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
    if (room == 0)
    {
        printf("No room with tag `START` found.\n");
        return -1;
    }
    char cmd_buffer[128];
    print_room(room, dungeon);
    while (1)
    {
        read_input(cmd_buffer);
        if (strncmp(cmd_buffer, "move ", 5) == 0)
        {
            enum Direction dir;
            char *input = trim_wspace(cmd_buffer + 5);
            if (strcmp(input, "north") == 0)
                dir = DIR_NORTH;
            else if (strcmp(input, "south") == 0)
                dir = DIR_SOUTH;
            else if (strcmp(input, "east") == 0)
                dir = DIR_EAST;
            else if (strcmp(input, "west") == 0)
                dir = DIR_WEST;
            else
            {
                printf("Unknown direction: `%s`\n", input);
                continue;
            }
            struct Room *new_room = 0;
            char *key = 0;
            for (int i = 0; i < room->num_exits; i++)
            {
                struct Exit e = room->exits[i];
                if (e.dir == dir)
                {
                    new_room = e.room;
                    key = e.key;
                    break;
                }
            }
            if (new_room == 0)
            {
                printf("No exit %s\n", fmt_dir(dir));
                continue;
            }
            if (key != 0)
            {
                int key_found = 0;
                for (struct Item *item = player.inventory; item != 0; item = item->next)
                {
                    if (strcmp(item->name, key) == 0)
                    {
                        key_found = 1;
                        break;
                    }
                }
                if (key_found == 0)
                {
                    printf("That exit is locked. Find %s.\n", key);
                    continue;
                }
            }
            room = new_room;
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
                        room->items = item->next;
                    else
                        prev->next = item->next;
                    item->next = player.inventory;
                    player.inventory = item;
                    break;
                }
                prev = item;
            }
        }
        else if (strncmp(cmd_buffer, "drop ", 5) == 0)
        {
            char *item_name = trim_wspace(cmd_buffer + 5);
            struct Item *prev = 0;
            for (struct Item *item = player.inventory; item != 0; item = item->next)
            {
                if (strcmp(item_name, item->name) == 0)
                {
                    if (prev == 0)
                        player.inventory = item->next;
                    else
                        prev->next = item->next;
                    item->next = room->items;
                    room->items = item;
                    break;
                }
                prev = item;
            }
        }
        else if (strncmp(cmd_buffer, "equip ", 6) == 0)
        {
            char *item_name = trim_wspace(cmd_buffer + 6);
            struct Item *prev = 0;
            int found_item = 0;
            for (struct Item *item = player.inventory; item != 0; item = item->next)
            {
                if (strcmp(item_name, item->name) == 0)
                {
                    // gonna equip the item
                    struct Item *slot = 0;
                    switch (item->type)
                    {
                    case IT_ARMOR_HEAD:
                        slot = player.equipment + EQ_HEAD;
                        break;
                    case IT_ARMOR_CHEST:
                        slot = player.equipment + EQ_CHEST;
                        break;
                    case IT_ARMOR_LEGS:
                        slot = player.equipment + EQ_LEGS;
                        break;
                    case IT_ARMOR_FEET:
                        slot = player.equipment + EQ_FEET;
                        break;
                    case IT_SHIELD:
                        slot = player.equipment + EQ_SHIELD;
                        break;
                    case IT_WEAPON:
                        slot = player.equipment + EQ_WEAPON;
                        break;
                    default:
                        break;
                    }
                    if (slot == 0)
                        continue;
                    if (item->grants != 0)
                    {
                        struct SpellPage *page = malloc(sizeof(struct SpellPage));
                        page->next = player.spellbook;
                        player.spellbook = page;
                        page->spell = item->grants;
                    }
                    struct Item tmp;
                    memcpy(&tmp, slot, sizeof(struct Item));
                    memcpy(slot, item, sizeof(struct Item));
                    slot->next = 0;
                    // if there was something in the slot
                    if (tmp.name[0])
                    {
                        struct SpellPage *prev = 0;
                        for (struct SpellPage *p = player.spellbook; p != 0; p = p->next)
                        {
                            if (p->spell == tmp.grants)
                            {
                                if (prev == 0)
                                    player.spellbook = p->next;
                                else
                                    prev->next = p->next;
                                free(p);
                                break;
                            }
                        }
                        slot = item->next;
                        memcpy(item, &tmp, sizeof(struct Item));
                        item->next = slot;
                    }
                    else
                    {
                        if (prev == 0)
                            player.inventory = item->next;
                        else
                            prev->next = item->next;
                        free(item);
                    }
                    found_item = 1;
                    break;
                }
                prev = item;
            }
            if (found_item)
                update_stats(&player);
            else
            {
                printf("Failed to find item: `%s`\n", item_name);
                continue;
            }
        }
        else if (strncmp(cmd_buffer, "consume ", 8) == 0)
        {
            char *item_name = trim_wspace(cmd_buffer + 8);
            struct Item *prev = 0;
            int found_item = 0;
            for (struct Item *item = player.inventory; item != 0; item = item->next)
            {
                if (strcmp(item_name, item->name) == 0 && item->type == IT_CONSUME)
                {
                    print_spell(item->grants);
                    confirm();
                    resolve_spell(item->grants, player.stats.mana, room, &player.stats);
                    // delete the item
                    if (prev == 0)
                        player.inventory = item->next;
                    else
                        prev->next = item->next;
                    free(item);
                    // say we found an item
                    found_item = 1;
                    break;
                }
                prev = item;
            }
            if (found_item == 0)
            {
                printf("Failed to find consumable item: `%s`\n", item_name);
                continue;
            }
        }
        else if (strncmp(cmd_buffer, "fight ", 5) == 0)
        {
            char *enemy_name = trim_wspace(cmd_buffer + 5);
            int found = 0;
            for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
            {
                if (strcmp(enemy->stats.name, enemy_name) == 0)
                {
                    fight(&player.stats, &enemy->stats);
                    found = 1;
                    confirm();
                    for (int i = 0; i < enemy->num_abilities; i++)
                    {
                        struct Ability ability = enemy->abilities[i];
                        if (ability.trigger != T_DEF)
                            continue;
                        resolve_ability(ability.result, enemy->stats.mana, room, &enemy->stats, &player.stats, 0);
                    }
                    for (int i = 0; i < NUM_EQ_SLOTS; i++)
                    {
                        if (player.equipment[i].name[0] == 0)
                            continue;
                        for (int j = 0; j < player.equipment[i].num_abilities; j++)
                        {
                            struct Ability ability = player.equipment[i].abilities[i];
                            if (ability.trigger != T_ATK)
                                continue;
                            resolve_ability(ability.result, player.stats.mana, room, &player.stats, &enemy->stats, 1);
                        }
                    }
                    break;
                }
            }
            if (!found)
            {
                printf("No such enemy: `%s`", enemy_name);
                continue;
            }
        }
        else if (strncmp(cmd_buffer, "cast ", 5) == 0)
        {
            char *spell_name = trim_wspace(cmd_buffer + 5);
            struct Spell *spell = 0;
            for (struct SpellPage *p = player.spellbook; p != 0; p = p->next)
            {
                if (strcmp(spell_name, p->spell->name) != 0)
                    continue;
                spell = p->spell;
                break;
            }
            if (spell == 0)
            {
                printf("You can't cast `%s`\n", spell_name);
                continue;
            }
            resolve_spell(spell, player.stats.mana, room, &player.stats);
        }
        else if (strcmp(cmd_buffer, "inv") == 0 || strcmp(cmd_buffer, "inventory") == 0)
        {
            printf("\n");
            for (int i = 0; i < NUM_EQ_SLOTS; i++)
                if (player.equipment[i].name[0])
                {
                    printf("%s:\n ", fmt_equip_slot(i));
                    print_item(&player.equipment[i]);
                }
            if (player.inventory)
            {
                printf("Inventory:\n");
                for (struct Item *item = player.inventory; item != 0; item = item->next)
                {
                    printf(" ");
                    print_item(item);
                }
            }
            continue;
        }
        else if (strcmp(cmd_buffer, "look") == 0)
        {
            print_room(room, dungeon);
            continue;
        }
        else if (strcmp(cmd_buffer, "stats") == 0)
        {
            printf("%s\nHP: %i/%i\nATK: %i", player.stats.name, player.stats.hp, player.stats.max_hp, player.stats.atk);
            if (player.stats.rage != 0)
                printf(" (%+i)", player.stats.rage);
            printf("\nDEF: %i", player.stats.def);
            if (player.stats.fortify != 0)
                printf(" (%+i)", player.stats.fortify);
            printf("\nMANA: %i\n", player.stats.mana);
            if (player.stats.burn > 0)
                printf("burn %i\n", player.stats.burn);
            if (player.stats.poison > 0)
                printf("poison %i\n", player.stats.poison);
            if (player.stats.regen > 0)
                printf("regen %i\n", player.stats.regen);
            if (player.stats.rage > 0)
                printf("rage %i\n", player.stats.rage);
            if (player.stats.rage < 0)
                printf("weakness %i\n", -player.stats.rage);
            if (player.stats.fortify > 0)
                printf("fortify %i\n", player.stats.fortify);
            if (player.stats.stun > 0)
                printf("stunned %i\n", player.stats.stun);
            continue;
        }
        else if (strcmp(cmd_buffer, "spellbook") == 0 || strcmp(cmd_buffer, "spells") == 0)
        {
            for (struct SpellPage *p = player.spellbook; p != 0; p = p->next)
                print_spell(p->spell);
            continue;
        }
        else if (strcmp(cmd_buffer, "q") == 0)
            break;
        else
        {
            printf("Unknown command\n");
            continue;
        }
        struct Enemy *prev = 0;
        for (struct Enemy *enemy = room->enemies; enemy != 0;)
        {
            tick(&enemy->stats);
            if (enemy->stats.hp <= 0)
            {
                printf("%s has perished.\n", enemy->stats.name);
                confirm();
                if (enemy->drops != 0)
                {
                    // find the end of the list of enemy drops
                    struct Item *last = enemy->drops;
                    while (last->next != 0)
                        last = last->next;
                    // prepend the list of enemy drops to the list of items in the room
                    last->next = room->items;
                    room->items = enemy->drops;
                }
                for (int i = 0; i < enemy->num_abilities; i++)
                {
                    struct Ability ability = enemy->abilities[i];
                    if (ability.trigger != T_DEATH)
                        continue;
                    resolve_ability(ability.result, enemy->stats.mana, room, &enemy->stats, 0, 0);
                }
                if (prev == 0)
                {
                    room->enemies = enemy->next;
                    free(enemy);
                    enemy = room->enemies;
                }
                else
                {
                    prev->next = enemy->next;
                    free(enemy);
                    enemy = prev->next;
                }
            }
            else
            {
                prev = enemy;
                enemy = enemy->next;
            }
        }
        tick(&player.stats);
        for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
        {
            fight(&enemy->stats, &player.stats);
            for (int i = 0; i < enemy->num_abilities; i++)
            {
                struct Ability ability = enemy->abilities[i];
                if (ability.trigger != T_ATK)
                    continue;
                resolve_ability(ability.result, enemy->stats.mana, room, &enemy->stats, &player.stats, 0);
            }
            for (int i = 0; i < NUM_EQ_SLOTS; i++)
            {
                if (player.equipment[i].name[0] == 0)
                    continue;
                for (int j = 0; j < player.equipment[i].num_abilities; j++)
                {
                    struct Ability ability = player.equipment[i].abilities[i];
                    if (ability.trigger != T_DEF)
                        continue;
                    resolve_ability(ability.result, player.stats.mana, room, &player.stats, &enemy->stats, 1);
                }
            }
            confirm();
        }
        for (int i = 0; i < NUM_EQ_SLOTS; i++)
        {
            if (player.equipment[i].name[0] == 0)
                continue;
            for (int j = 0; j < player.equipment[i].num_abilities; j++)
            {
                struct Ability ability = player.equipment[i].abilities[i];
                if (ability.trigger != T_TURN)
                    continue;
                resolve_ability(ability.result, player.stats.mana, room, &player.stats, 0, 1);
            }
        }
        for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
        {
            for (int i = 0; i < enemy->num_abilities; i++)
            {
                struct Ability ability = enemy->abilities[i];
                if (ability.trigger != T_TURN)
                    continue;
                resolve_ability(ability.result, enemy->stats.mana, room, &enemy->stats, 0, 0);
            }
        }
        if (player.stats.hp <= 0)
        {
            printf("%s has perished.\n", player.stats.name);
            confirm();
            return 0;
        }
        print_room(room, dungeon);
    }
}

char *fmt_dir(enum Direction dir)
{
    switch (dir)
    {
    case DIR_NORTH:
        return "north";
    case DIR_SOUTH:
        return "south";
    case DIR_EAST:
        return "east";
    case DIR_WEST:
        return "west";
    default:
        return "ERROR";
    }
}

char *fmt_equip_slot(enum Equipment s)
{
    switch (s)
    {
    case EQ_HEAD:
        return "HEAD";
    case EQ_CHEST:
        return "CHEST";
    case EQ_LEGS:
        return "LEGS";
    case EQ_FEET:
        return "FEET";
    case EQ_WEAPON:
        return "WEAPON";
    case EQ_SHIELD:
        return "SHIELD";
    default:
        return 0;
    }
}

char *fmt_ability_trigger(enum Trigger t)
{
    switch (t)
    {
    case T_ATK:
        return "attack";
    case T_DEF:
        return "defend";
    case T_TURN:
        return "turn";
    case T_DEATH:
        return "death";
    default:
        return 0;
    }
}
