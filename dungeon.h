typedef enum Direction
{
    NORTH,
    SOUTH,
    EAST,
    WEST
};

typedef struct Enemy
{
    char *name;
};

typedef struct Item
{
    char *name;
};

typedef struct Exit
{
    int room;
    enum Direction dir;
};

typedef struct Room
{
    char *name;
    char *desc;
    int num_exits;
    int num_enemies;
    int num_items;
    struct Enemy *enemies;
    struct Item *items;
    struct Exit *exits;
};

typedef struct Dungeon
{
    int num_rooms;
    struct Room rooms[];
};

struct Dungeon *load_dungeon(char *);
