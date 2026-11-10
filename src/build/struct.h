#ifndef GAME_STRUCTS
#define GAME_STRUCTS

bool IsStructType(Type type) { return type > 35 && type < 36; }

struct Debug_Struct_Member {
    const char* name;
    Type struct_type;
    Type member_type;
    uint64 size;
    uint64 offset;
    int array_size;
    bool is_pointer;
};

const int STRUCT_MEMBERS_SIZE = 0;
Debug_Struct_Member* struct_members = 0;


#endif