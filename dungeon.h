enum Direction
{
    NORTH,
    SOUTH,
    EAST,
    WEST
};

struct Enemy
{
    char *name;
    int hp;
    struct Enemy *next;
};

struct Item
{
    char *name;
    struct Item *next;
};

struct Exit
{
    int room;
    enum Direction dir;
};

struct Room
{
    char *name;
    char *desc;
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
