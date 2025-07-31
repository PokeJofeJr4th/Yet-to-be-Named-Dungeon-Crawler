enum Direction
{
    NORTH,
    SOUTH,
    EAST,
    WEST
};
struct Combatant
{
    char name[32];
    int hp;
    int atk;
    int def;
};

struct Enemy
{
    struct Combatant stats;
    struct Enemy *next;
};

struct Item
{
    char name[32];
    struct Item *next;
};

struct Exit
{
    int room;
    enum Direction dir;
};

struct Room
{
    char name[32];
    char desc[32];
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

char *fmt_dir(enum Direction);
char *trim_wspace(char *);
struct Dungeon *load_dungeon(char *);
