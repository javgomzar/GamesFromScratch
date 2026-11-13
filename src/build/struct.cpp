#include "pch.h"
#include "struct.h"


bool IsStructType(Type type) { return type > 35 && type < 37; }

const Debug_Struct_Member struct_members[STRUCT_MEMBERS_SIZE] = {
    {"rotation", Type_Transform, Type_Quaternion, sizeof(Quaternion), (uint64)(&((Transform*)0)->rotation),0, false},
    {"translation", Type_Transform, Type_V3, sizeof(V3), (uint64)(&((Transform*)0)->translation),0, false},
    {"scale", Type_Transform, Type_Scale, sizeof(Scale), (uint64)(&((Transform*)0)->scale),0, false},
};
