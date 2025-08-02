enum Direction
{
    DIR_NORTH,
    DIR_SOUTH,
    DIR_EAST,
    DIR_WEST
};

struct Combatant
{
    char name[32];
    int hp;
    int atk;
    int def;
    int mana;
};

enum SpellTargetType
{
    ST_TARGET_ENEMY,
    ST_EACH_ENEMY,
    ST_TARGET_ALLY,
    ST_EACH_ALLY,
    ST_SELF
};

enum SpellEffectType
{
    SE_DMG,
    SE_HEAL,
    SE_RAGE,
    SE_FORTIFY,
    SE_MANA,
    SE_BURN,
    SE_POISON,
    SE_STUN,
    SE_REGEN,
};

struct SpellEffect
{
    int amount;
    enum SpellEffectType type;
};

struct SpellTarget
{
    struct SpellEffect *effects;
    int num_effects;
    enum SpellTargetType target;
};

struct Spell
{
    char name[32];
    struct SpellTarget *targets;
    char **tags;
    int num_targets;
    int num_tags;
};

struct Enemy
{
    struct Combatant stats;
    struct Enemy *next;
};

enum ItemType
{
    IT_DEFAULT,
    IT_WEAPON,
    IT_ARMOR_HEAD,
    IT_ARMOR_CHEST,
    IT_ARMOR_LEGS,
    IT_ARMOR_FEET,
    IT_SHIELD,
};

struct Item
{
    char name[32];
    struct Item *next;
    int atk;
    int def;
    int mana;
    enum ItemType type;
};

struct Exit
{
    int room;
    enum Direction dir;
};

struct Room
{
    char name[32];
    char desc[128];
    int num_exits;
    int num_tags;
    struct Enemy *enemies;
    struct Item *items;
    struct Exit *exits;
    char **tags;
};

struct Dungeon
{
    int num_rooms;
    struct Room rooms[];
};

struct Player
{
    struct Combatant stats;
    struct Item *inventory;
    // these fields are all optional. If the item name is empty, there's no item.
    struct Item head;
    struct Item chest;
    struct Item legs;
    struct Item feet;
    struct Item weapon;
    struct Item shield;
};

char *fmt_dir(enum Direction);
char *trim_wspace(char *);
struct Dungeon *load_dungeon(char *);
void fight(struct Combatant *attacker, struct Combatant *target);
