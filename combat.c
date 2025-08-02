#include "dungeon.h"
#include <stdio.h>

int take_damage(struct Combatant *target, int dmg)
{
    if (dmg <= 0)
    {
        dmg = 1;
    }
    if (dmg > target->hp)
    {
        dmg = target->hp;
    }
    target->hp -= dmg;
    return dmg;
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
    target->hp -= dmg;
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
        c->hp += c->regen--;
    if (c->fortify)
        c->fortify--;
    if (c->rage != 0)
        c->rage -= sign(c->rage);
    if (c->stun)
        c->stun--;
}

void apply_effect(struct SpellEffect *effect, int mana, struct Combatant *target)
{
    int magnitude = effect->amount + mana;
    switch (effect->type)
    {
    case SE_BURN:
        target->burn += magnitude;
        break;
    case SE_FORTIFY:
        target->fortify += magnitude;
        break;
    case SE_HEAL:
        target->hp += magnitude;
        break;
    case SE_POISON:
        target->poison += magnitude;
        break;
    case SE_REGEN:
        target->regen += magnitude;
        break;
    case SE_STUN:
        target->stun += magnitude;
        break;
    case SE_WEAK:
        target->rage -= magnitude;
        break;
    case SE_DMG:
        take_damage(target, magnitude);
        break;
    default:
        break;
    }
}

void resolve_spell(struct Spell *spell, int mana)
{
}
