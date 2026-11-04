#ifndef GAME_STRUCTS
#define GAME_STRUCTS

bool IsStructType(Type type) { return type > 35 && type < 38; }

struct Debug_Struct_Member {
    const char* name;
    Type struct_type;
    Type member_type;
    uint64 size;
    uint64 offset;
    int array_size;
    bool is_pointer;
};

const int STRUCT_MEMBERS_SIZE = 5;
Debug_Struct_Member struct_members[STRUCT_MEMBERS_SIZE] = {
    {"translation", Type_Transform, Type_V3, sizeof(V3), (uint64)(&((Transform*)0)->translation),0, false},
    {"scale", Type_Transform, Type_Scale, sizeof(Scale), (uint64)(&((Transform*)0)->scale),0, false},
    {"rotation", Type_Transform, Type_Quaternion, sizeof(Quaternion), (uint64)(&((Transform*)0)->rotation),0, false},
    {"head", Type_Segment3, Type_V3, sizeof(V3), (uint64)(&((Segment3*)0)->head),0, false},
    {"tail", Type_Segment3, Type_V3, sizeof(V3), (uint64)(&((Segment3*)0)->tail),0, false},
};

#endif