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
    int max_hp;
    int atk;
    int def;
    int mana;
    int rage;
    int fortify;
    int burn;
    int poison;
    int regen;
    int stun;
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
    SE_WEAK,
    SE_FORTIFY,
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
    enum SpellTargetType type;
};

struct Spell
{
    char name[32];
    struct SpellTarget *targets;
    char **tags;
    int num_targets;
    int num_tags;
    int cost;
};

struct Enemy
{
    struct Combatant stats;
    struct Enemy *next;
    struct Item *drops;
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
    IT_CONSUME,
};

struct Item
{
    char name[32];
    struct Item *next;
    int atk;
    int def;
    int mana;
    struct Spell *grants;
    enum ItemType type;
};

struct Exit
{
    struct Room *room;
    char *key;
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
    struct Spell *spells;
    int num_rooms;
    int num_spells;
    struct Room rooms[];
};

struct SpellPage
{
    struct Spell *spell;
    struct SpellPage *next;
};

struct Player
{
    struct Combatant stats;
    struct Item *inventory;
    struct SpellPage *spellbook;
    // these fields are all optional. If the item name is empty, there's no item.
    struct Item head;
    struct Item chest;
    struct Item legs;
    struct Item feet;
    struct Item weapon;
    struct Item shield;
};

// I/O Helper Functions
char *fmt_dir(enum Direction);
char *trim_wspace(char *);
void confirm();
void read_input(char *buffer);

// Dungeon Initialization
struct Dungeon *load_dungeon(char *);

// Combat
void fight(struct Combatant *attacker, struct Combatant *target);
void resolve_spell(struct Spell *spell, int mana, struct Room *room, struct Combatant *caster);
void tick(struct Combatant *c);
