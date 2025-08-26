enum debug_type {
    Debug_Type_bool,
    Debug_Type_char,
    Debug_Type_string,
    Debug_Type_int8,
    Debug_Type_int16,
    Debug_Type_int,
    Debug_Type_int32,
    Debug_Type_int64,
    Debug_Type_uint8,
    Debug_Type_uint16,
    Debug_Type_uint32,
    Debug_Type_uint64,
    Debug_Type_memory_index,
    Debug_Type_float,
    Debug_Type_double,
    Debug_Type_v2,
    Debug_Type_v3,
    Debug_Type_v4,
    Debug_Type_scale,
    Debug_Type_quaternion,
    Debug_Type_color,
    Debug_Type_collider,
    Debug_Type_memory_arena,
    Debug_Type_game_entity_type,
    Debug_Type_character_action_id,
    Debug_Type_transform,
    Debug_Type_game_entity,
};

bool IsEnumType(debug_type Type) { return Type > 22 && Type < 25; }
bool IsStructType(debug_type Type) { return Type > 24 && Type < 27; }

struct debug_enum_value {
    debug_type EnumType;
    char* Identifier;
    int Value;
};

const int ENUM_VALUES_SIZE = 10;
debug_enum_value EnumValues[ENUM_VALUES_SIZE] = {
    {Debug_Type_game_entity_type, "Entity_Type_Character", 0},
    {Debug_Type_game_entity_type, "Entity_Type_Enemy", 1},
    {Debug_Type_game_entity_type, "Entity_Type_Camera", 2},
    {Debug_Type_game_entity_type, "Entity_Type_Prop", 3},
    {Debug_Type_game_entity_type, "Entity_Type_Weapon", 4},
    {Debug_Type_game_entity_type, "game_entity_type_count", 5},
    {Debug_Type_character_action_id, "Character_Action_Idle_ID", 0},
    {Debug_Type_character_action_id, "Character_Action_Walk_ID", 1},
    {Debug_Type_character_action_id, "Character_Action_Jump_ID", 2},
    {Debug_Type_character_action_id, "Character_Action_Attack_ID", 3},
};

struct debug_struct_member {
    char* Name;
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
