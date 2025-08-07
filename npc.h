enum NpcOpcode
{
    OPC_IF_HAS,
    OPC_UNLESS_HAS,
    OPC_IF_FLAG,
    OPC_UNLESS_FLAG,
    OPC_SET_FLAG,
    OPC_CLEAR_FLAG,
    OPC_SAY,
    OPC_STATE,
    OPC_CAST,
    OPC_DROP,
    OPC_SUMMON,
    OPC_DESPAWN
};

struct NpcInstruction
{
    enum NpcOpcode opcode;
    union
    {
        // used for *_HAS, SAY
        char *text;
        // used for CAST
        struct Spell *spell;
        // used for SUMMON
        struct Enemy *enemy;
        // used for DROP
        struct Item *item;
        // used for *_FLAG and STATE
        int id;
    } data;
};

struct NpcOption
{
    char text[32];
    struct NpcInstruction instructions[8];
};

struct NpcState
{
    struct NpcInstruction instructions[16];
    struct NpcOption options[8];
};

struct Npc
{
    char name[32];
    struct Npc *next;
    struct NpcState *states;
    int num_states;
    int flags;
};