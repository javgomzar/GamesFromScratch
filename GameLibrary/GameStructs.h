bool IsStructType(debug_type Type) { return Type > 35 && Type < 37; }

struct debug_struct_member {
    const char* Name;
    debug_type StructType;
    debug_type MemberType;
    uint64 Size;
    uint64 Offset;
    int ArraySize;
    bool IsPointer;
};

const int STRUCT_MEMBERS_SIZE = 3;
debug_struct_member StructMembers[STRUCT_MEMBERS_SIZE] = {
    {"Translation", Debug_Type_transform, Debug_Type_v3, sizeof(v3), (uint64)(&((transform*)0)->Translation),0, false},
    {"Scale", Debug_Type_transform, Debug_Type_scale, sizeof(scale), (uint64)(&((transform*)0)->Scale),0, false},
    {"Rotation", Debug_Type_transform, Debug_Type_quaternion, sizeof(quaternion), (uint64)(&((transform*)0)->Rotation),0, false},
};
