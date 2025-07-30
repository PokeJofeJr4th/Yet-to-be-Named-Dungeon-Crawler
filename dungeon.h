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

typedef struct Room
{
    int num_exits;
    int num_enemies;
    int num_items;
    struct Enemy *enemies;
    struct Item *items;
    struct
    {
        int exit_to;
        enum Direction exit_dir;
    } exits[];
};

typedef struct Dungeon
{
    int num_rooms;
    struct Room *rooms[];
};
    
struct Dungeon *load_dungeon(char *);
