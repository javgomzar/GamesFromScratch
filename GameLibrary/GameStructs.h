bool IsStructType(debug_type Type) { return Type > 38 && Type < 41; }

struct debug_struct_member {
    const char* Name;
    debug_type StructType;
    debug_type MemberType;
    uint64 Size;
    uint64 Offset;
    int ArraySize;
    bool IsPointer;
};

const int STRUCT_MEMBERS_SIZE = 14;
debug_struct_member StructMembers[STRUCT_MEMBERS_SIZE] = {
    {"Translation", Debug_Type_transform, Debug_Type_v3, sizeof(v3), (uint64)(&((transform*)0)->Translation),0, false},
    {"Scale", Debug_Type_transform, Debug_Type_scale, sizeof(scale), (uint64)(&((transform*)0)->Scale),0, false},
    {"Rotation", Debug_Type_transform, Debug_Type_quaternion, sizeof(quaternion), (uint64)(&((transform*)0)->Rotation),0, false},
    {"Name", Debug_Type_game_entity, Debug_Type_string, sizeof(char), (uint64)(&((game_entity*)0)->Name),0, false},
    {"ID", Debug_Type_game_entity, Debug_Type_int, sizeof(int), (uint64)(&((game_entity*)0)->ID),0, false},
    {"Parent", Debug_Type_game_entity, Debug_Type_game_entity, sizeof(game_entity), (uint64)(&((game_entity*)0)->Parent),0, true},
    {"Index", Debug_Type_game_entity, Debug_Type_int, sizeof(int), (uint64)(&((game_entity*)0)->Index),0, false},
    {"Type", Debug_Type_game_entity, Debug_Type_game_entity_type, sizeof(game_entity_type), (uint64)(&((game_entity*)0)->Type),0, false},
    {"Transform", Debug_Type_game_entity, Debug_Type_transform, sizeof(transform), (uint64)(&((game_entity*)0)->Transform),0, false},
    {"Velocity", Debug_Type_game_entity, Debug_Type_v3, sizeof(v3), (uint64)(&((game_entity*)0)->Velocity),0, false},
    {"Collider", Debug_Type_game_entity, Debug_Type_collider, sizeof(collider), (uint64)(&((game_entity*)0)->Collider),0, false},
    {"Collided", Debug_Type_game_entity, Debug_Type_bool, sizeof(bool), (uint64)(&((game_entity*)0)->Collided),0, false},
    {"Active", Debug_Type_game_entity, Debug_Type_bool, sizeof(bool), (uint64)(&((game_entity*)0)->Active),0, false},
    {"Hovered", Debug_Type_game_entity, Debug_Type_bool, sizeof(bool), (uint64)(&((game_entity*)0)->Hovered),0, false},
};
