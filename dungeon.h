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
