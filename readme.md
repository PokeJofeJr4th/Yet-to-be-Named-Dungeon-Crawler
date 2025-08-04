# Yet-to-be-Named Dungeon Crawler

## How to Play

Once it's all compiled (see Makefile), use `./dungeon.exe` to run the default dungeon, or `./dungeon.exe my_dungeon.txt` if you make your own or download somebody else's.

This is a command-based game. Upon loading into the world, you will see the output of the `look` command. You should familiarize yourself with the following actions you can take in the game.

### Look

Ex. `look`

Displays information about the room you are currently in. Example output is shown below.

```
Entrance Hall
 A cold stone chamber with dripping moss.
Enemies:
 Goblin Scout (2/2 HP, 1 ATK, 0 DEF)
Items:
 Rusty Sword (weapon: +2 ATK)
 Silver Circlet (head: +1 MANA <Firebolt>)
 Healing Potion (consume: <Heal>)
Exits:
 north: Great Hall
```

You can use the other commands to fight enemies, interact with items, and explore other rooms.

### Move

Ex. `move north`

Moves your character through an exit in the specified direction. The direction must correspond to an exit shown in the current room. If a room is locked, you won't be able to move through until you have the key in your inventory.

### Take/Drop

Ex. `take Rusty Sword`, `drop Rusty Sword`

These commands transfer items between your inventory and the room you're currently in. There's no penalty for having too many items yet, so this is just used to tidy up.

### Equip

Ex. `equip Silver Circlet`

Moves an item from your inventory to an equipment slot. The slots are head, chest, legs, feet, weapon, and shield. The slot an item fits into is shown on the `look` and `inv` screens. If you already have an item of a certain type equipped, the old item is automatically placed into your inventory.

### Consume

Ex. `consume Healing Potion`

Consumes a consumable item in your inventory. On the `look` and `inv` screen, consumable items show the spell they cast in `<Angle Brackets>`. Spells cast from consumable items follow all the same rules as from the `cast` command.

### Inv

Ex. `inv`

Displays the contents of your inventory and any items you currently have equipped.

### Stats

Ex. `stats`

Displays your stats, including HP, ATK, DEF, Mana, and any other status effects you are under.

**HP** is HP.

**ATK** is damage dealt with a basic attack.

**DEF** reduces damage from incoming attacks. Damage cannot be reduced below 1.

**MANA** boosts your spells' damage, healing, and status effects.

**Rage** increases ATK while decaying over time. Negative Rage is also called Weakness.

**Fortify** increases DEF while decaying over time.

**Burn** deals damage at the end of each turn while decaying over time.

**Poison** deals damage at the end of each turn while decaying over time.

**Regen** heals at the end of each turn while decaying over time.

### Fight

Ex. `fight Goblin Scout`

Attack an enemy in the room.

### Spellbook

Ex. `spellbook`, `spells`

Display the effects of all spells you have access to via equipped items. Example output is shown below.

```
Firebolt (cost: 1)
 Tags: FIRE
 Target an Enemy:
  Apply burn 2
  Deal 2 damage
```

**Cost** does nothing yet.

**Tags** do nothing yet.

**Target Blocks** start with a phrase like "Target an Enemy", "Self", or "All Allies", describing which entities the target block affects. The indented lines afterward describe the damage, healing, and status effects that apply.

### Cast

Ex. `cast Firebolt`

Cast a spell. You will be asked to select targets, and then the spell's effects will be resolved. You can only cast a spell if it is granted by an item you currently have equipped. If a target needs to be chosen, you can cancel the target block by pressing enter.

## How to Make a Dungeon

A dungeon is a text file containing definitions of spells and rooms. Spaces and tabs are largely ignored, but indentation is recommended to make scope easier to follow. It is also recommended to place lines in the order described here. Certain other orderings can produce unexpected behavior.

Once you're done with this guide, you can look at `example.txt` to see what a dungeon file looks like.

### Spells

```
SPELL    Firebolt
 TAG     FIRE
 COST    1
 TARGET  ENEMY
  DMG    2
  EFFECT BURN 2
```

The first line specifies the name of the spell, which is used later by items. `COST` and `TAGS` do not have any gameplay impact yet.

Next are target blocks. These determine how a spell determines which entities it effects. `TARGET SELF` will affect the caster, `TARGET ENEMY` will allow the caster to choose an enemy to target, and `EACH ENEMY` will affect all enemies.

Inside the target block are effects. The available spell effects are `DMG {amount}`, `HEAL {amount}`, and `APPLY {STATUS} {amount}`. All numbers must be positive. When balancing spells, you should keep in mind that the caster's MANA stat is used to increase all amounts in this section. Below is a list of status effects. They correspond to the effects described in the **Stats** section above.

- RAGE
- BURN
- POISON
- WEAK
- FORTIFY
- REGEN

A spell can have multiple target blocks, and their effects are determined independently.

### Rooms

```
ROOM    Entrance Hall
 DESC   A cold stone chamber with dripping moss.
 TAG    START
 ITEM   Rusty Sword
  EQUIP WEAPON
  ATK   2
 ITEM   Bronze Chestplate
  EQUIP CHEST
  DEF   1
 ENEMY  Goblin Scout
  HP    2
 EXIT   NORTH Great Hall
```

The first line specifies the name of the room, which is used in later exits. The second line is an optional, flavorful description. Afterwards are a series of tags. Currently, the only useful tag is `START`; every dungeon must have exactly one room with that tag for the player to begin their adventure.

### Items

```
 ITEM     Healing Potion
  CONSUME Heal
 ITEM     Silver Circlet
  EQUIP   HEAD
  MANA    1
  GRANTS  Firebolt
```

The first line of an item specifies its name, which is used by the player to identify it. The next line specifies what type of item it is.

An item with `CONSUME {Spell Name}` is a consumable item which casts the indicated spell when it is consumed (see the **Consume** section above).

An item with `EQUIP {SLOT}` can be equipped by the player. The slots are `HEAD`, `CHEST`, `LEGS`, `FEET`, `WEAPON`, and `SHIELD`. These items can have further properties, described below.

A piece of equipment with `GRANTS {Spell Name}` allows the player to cast the specified spell while the item is equipped (see the **Cast** section above).

A piece of equipment with `ATK {amount}`, `DEF {amount}`, or `MANA {amount}` increases the player's corresponding stat by the specified amount when worn. ATK causes the wearer to deal more damage in combat (recommended for weapons), DEF causes the wearer to take less damage in combat (recommended for armor), and MANA causes the wearer's spells to become more potent (recommended for magical items).

### Enemies

```
ENEMY  Cult Acolyte
  HP   3
  ATK  1
  DEF  1
```

An enemy block starts with its name. This is used by the player to target the enemy for spells and attacks, so it should be unique within the room. Then, the enemy's HP, ATK, and DEF stats are specified. These are all optional, and the defaults are 1 HP, 1 ATK, and 0 DEF.

### Exits

```
EXIT EAST Crypt
 KEY Crypt Key
```

An exit follows the pattern `EXIT {DIRECTION} {Room Name}`. The direction must be one of `NORTH`, `SOUTH`, `EAST`, and `WEST`. There can only be one exit in each direction. It is highly recommended to add exits in pairs between adjacent rooms; if room A has `EXIT SOUTH B`, room B should have `EXIT NORTH A`.

An exit with a key checks if an item with the specified name is in the player's inventory before allowing them through. It is recommended to add a key to both sides of an exit to prevent the player from getting stuck. Make sure you put the key somewhere accessible, and don't make it equipment or consumable.
