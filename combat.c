#include "dungeon.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int take_damage(struct Combatant *target, int dmg)
{
    if (dmg <= 0)
        dmg = 1;
    if (dmg > target->hp)
        dmg = target->hp;
    target->hp -= dmg;
    return dmg;
}

int heal(struct Combatant *target, int healing)
{
    if (healing <= 0)
        healing = 1;
    if (healing + target->hp > target->max_hp)
        healing = target->max_hp - target->hp;
    target->hp += healing;
    return healing;
}

void fight(struct Combatant *attacker, struct Combatant *target)
{
    if (attacker->stun)
    {
        printf("%s is stunned and can't attack!", attacker->name);
        return;
    }
    int atk = attacker->atk + attacker->rage;
    int def = target->def + target->fortify;
    int dmg = take_damage(target, atk - def);
    printf("%s attacks %s, dealing %i damage.\n", attacker->name, target->name, dmg);
}

int sign(int x)
{
    if (x == 0)
        return 0;
    if (x > 0)
        return 1;
    return -1;
}

void tick(struct Combatant *c)
{
    if (c->burn)
        printf("%s takes %i damage from burning.\n", c->name, take_damage(c, c->burn--));
    if (c->poison)
        printf("%s takes %i damage from poison.\n", c->name, take_damage(c, c->poison--));
    if (c->regen)
        printf("%s heals %i HP from regeneration.\n", c->name, heal(c, c->regen--));
    if (c->fortify)
        c->fortify--;
    if (c->rage != 0)
        c->rage -= sign(c->rage);
    if (c->stun)
        c->stun--;
}

void apply_effect(struct SpellStatus *effect, int mana, struct Combatant *target)
{
    int magnitude = effect->amount + mana;
    switch (effect->type)
    {
    case SE_BURN:
        target->burn += magnitude;
        printf("%s gains Burn %i.\n", target->name, magnitude);
        break;
    case SE_FORTIFY:
        target->fortify += magnitude;
        printf("%s gains Fortify %i.\n", target->name, magnitude);
        break;
    case SE_HEAL:
        printf("%s regains %i HP.\n", target->name, heal(target, magnitude));
        break;
    case SE_POISON:
        target->poison += magnitude;
        printf("%s gains Poison %i.\n", target->name, magnitude);
        break;
    case SE_REGEN:
        target->regen += magnitude;
        printf("%s gains Regeneration %i.\n", target->name, magnitude);
        break;
    case SE_STUN:
        target->stun += magnitude;
        printf("%s gains Stunned %i.\n", target->name, magnitude);
        break;
    case SE_WEAK:
        target->rage -= magnitude;
        printf("%s gains Weakness %i.\n", target->name, magnitude);
        break;
    case SE_DMG:
        printf("%s takes %i damage.\n", target->name, take_damage(target, magnitude));
        break;
    default:
        break;
    }
}

struct Item *clone_items(struct Item *s)
{
    if (s == 0)
        return 0;
    struct Item *d = malloc(sizeof(struct Item));
    memcpy(d, s, sizeof(struct Item));
    d->abilities = malloc(sizeof(struct Ability) * d->num_abilities);
    memcpy(d->abilities, s->abilities, sizeof(struct Ability) * d->num_abilities);
    d->next = clone_items(s->next);
    return d;
}

struct Enemy *clone_enemy(struct Enemy *s)
{
    struct Enemy *d = malloc(sizeof(struct Enemy));
    memcpy(d, s, sizeof(struct Enemy));
    d->abilities = malloc(sizeof(struct Ability) * d->num_abilities);
    memcpy(d->abilities, s->abilities, sizeof(struct Ability) * d->num_abilities);
    d->drops = clone_items(s->drops);
    return d;
}

void summon(struct Enemy *enemy, struct Room *room)
{
    enemy = clone_enemy(enemy);
    printf("%s has been summoned.\n", enemy->stats.name);
    enemy->next = 0;
    if (room->enemies == 0)
        room->enemies = enemy;
    else
    {
        struct Enemy *tail = 0;
        for (struct Enemy *e = room->enemies; e != 0; e = e->next)
            tail = e;
        tail->next = enemy;
    }
}

void resolve_spell(struct Spell *spell, int mana, struct Room *room, struct Combatant *caster)
{
    for (int i = 0; i < spell->num_blocks; i++)
    {
        struct SpellBlock block = spell->blocks[i];
        switch (block.type)
        {
        case ST_SELF:
        case ST_EACH_ALLY:
        case ST_TARGET_ALLY:
            for (int j = 0; j < block.effect.status.num_effects; j++)
                apply_effect(&block.effect.status.effects[j], mana, caster);
            break;
        case ST_EACH_ENEMY:
            for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
                for (int j = 0; j < block.effect.status.num_effects; j++)
                    apply_effect(&block.effect.status.effects[j], mana, &enemy->stats);
            break;
        case ST_TARGET_ENEMY:
            char buffer[128];
            printf("Select an enemy to target");
            read_input(buffer);
            for (int i = strlen(buffer) - 1; i > 0 && isspace(buffer[i]); buffer[i--] = '\0')
                ;
            char *line = trim_wspace(buffer);
            for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
            {
                if (strcmp(enemy->stats.name, line) != 0)
                    continue;
                for (int j = 0; j < block.effect.status.num_effects; j++)
                    apply_effect(&block.effect.status.effects[j], mana, &enemy->stats);
                break;
            }
            break;
        case ST_SUMMON:
            summon(block.effect.summon, room);
            break;
        }
        check_deaths(room);
    }
}

void resolve_ability(struct Spell *spell, int mana, struct Room *room, struct Combatant *source, struct Combatant *opponent, int is_player)
{
    for (int i = 0; i < spell->num_blocks; i++)
    {
        struct SpellBlock block = spell->blocks[i];
        switch (block.type)
        {
        case ST_EACH_ALLY:
            if (!is_player)
            {
                for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
                    for (int j = 0; j < block.effect.status.num_effects; j++)
                        apply_effect(&block.effect.status.effects[j], mana, &enemy->stats);
                break;
            }
        case ST_SELF:
        case ST_TARGET_ALLY:
            for (int j = 0; j < block.effect.status.num_effects; j++)
                apply_effect(&block.effect.status.effects[j], mana, source);
            break;
        case ST_EACH_ENEMY:
            if (is_player)
            {
                for (struct Enemy *enemy = room->enemies; enemy != 0; enemy = enemy->next)
                    for (int j = 0; j < block.effect.status.num_effects; j++)
                        apply_effect(&block.effect.status.effects[j], mana, &enemy->stats);
                break;
            }
        case ST_TARGET_ENEMY:
            for (int j = 0; j < block.effect.status.num_effects; j++)
                apply_effect(&block.effect.status.effects[j], mana, opponent);
            break;
        case ST_SUMMON:
            summon(block.effect.summon, room);
            break;
        }
    }
}

void check_deaths(struct Room *room)
{
    int death;
    do
    {
        death = 0;
        struct Enemy *prev = 0;
        for (struct Enemy *enemy = room->enemies; enemy != 0;)
        {
            if (enemy->stats.hp <= 0)
            {
                printf("%s has perished.\n", enemy->stats.name);
                death++;
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
    } while (death);
}