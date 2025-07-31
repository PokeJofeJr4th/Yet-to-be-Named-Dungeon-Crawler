#include "dungeon.h"
#include <stdio.h>

void fight(struct Combatant *attacker, struct Combatant *target)
{
    int dmg = attacker->atk - target->def;
    if (dmg <= 0)
    {
        dmg = 1;
    }
    if (dmg > target->hp)
    {
        dmg = target->hp;
    }
    printf("%s attacks %s, dealing %i damage.\n", attacker->name, target->name, dmg);
    target->hp -= dmg;
}