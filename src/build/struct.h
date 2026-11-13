#pragma once

#include "enum.h"

bool IsStructType(Type type);

struct Debug_Struct_Member {
    const char* name;
    Type struct_type;
    Type member_type;
    uint64 size;
    uint64 offset;
    int array_size;
    bool is_pointer;
};

inline constexpr int STRUCT_MEMBERS_SIZE = 3;
extern const Debug_Struct_Member struct_members[STRUCT_MEMBERS_SIZE];
