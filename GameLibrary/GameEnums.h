#ifndef GAME_ENUMS
#define GAME_ENUMS

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
    Debug_Type_game_asset_type,
    Debug_Type_game_text_id,
    Debug_Type_game_heightmap_id,
    Debug_Type_game_animation_id,
    Debug_Type_game_entity_type,
    Debug_Type_weapon_type,
    Debug_Type_character_action_id,
    Debug_Type_transform,
    Debug_Type_game_entity,
};

bool IsEnumType(debug_type Type) { return Type > 22 && Type < 30; }

struct debug_enum_value {
    debug_type EnumType;
    const char* Identifier;
    int Value;
};

const int ENUM_VALUES_SIZE = 32;
debug_enum_value EnumValues[ENUM_VALUES_SIZE] = {
    {Debug_Type_game_asset_type, "Asset_Type_Text", 0},
    {Debug_Type_game_asset_type, "Asset_Type_Bitmap", 1},
    {Debug_Type_game_asset_type, "Asset_Type_Heightmap", 2},
    {Debug_Type_game_asset_type, "Asset_Type_Font", 3},
    {Debug_Type_game_asset_type, "Asset_Type_Sound", 4},
    {Debug_Type_game_asset_type, "Asset_Type_Video", 5},
    {Debug_Type_game_asset_type, "Asset_Type_Mesh", 6},
    {Debug_Type_game_asset_type, "Asset_Type_Animation", 7},
    {Debug_Type_game_asset_type, "game_asset_type_count", 8},
    {Debug_Type_game_text_id, "Text_Test_ID", 0},
    {Debug_Type_game_text_id, "game_text_id_count", 1},
    {Debug_Type_game_heightmap_id, "Heightmap_Spain_ID", 0},
    {Debug_Type_game_heightmap_id, "game_heightmap_id_count", 1},
    {Debug_Type_game_animation_id, "Animation_Idle_ID", 0},
    {Debug_Type_game_animation_id, "Animation_Walk_ID", 1},
    {Debug_Type_game_animation_id, "Animation_Jump_ID", 2},
    {Debug_Type_game_animation_id, "Animation_Attack_ID", 3},
    {Debug_Type_game_animation_id, "game_animation_id_count", 4},
    {Debug_Type_game_entity_type, "Entity_Type_Character", 0},
    {Debug_Type_game_entity_type, "Entity_Type_Enemy", 1},
    {Debug_Type_game_entity_type, "Entity_Type_Camera", 2},
    {Debug_Type_game_entity_type, "Entity_Type_Prop", 3},
    {Debug_Type_game_entity_type, "Entity_Type_Weapon", 4},
    {Debug_Type_game_entity_type, "game_entity_type_count", 5},
    {Debug_Type_weapon_type, "Weapon_Sword", 0},
    {Debug_Type_weapon_type, "Weapon_Shield", 1},
    {Debug_Type_weapon_type, "weapon_type_count", 2},
    {Debug_Type_character_action_id, "Character_Action_Idle_ID", 0},
    {Debug_Type_character_action_id, "Character_Action_Walk_ID", 1},
    {Debug_Type_character_action_id, "Character_Action_Jump_ID", 2},
    {Debug_Type_character_action_id, "Character_Action_Attack_ID", 3},
    {Debug_Type_character_action_id, "character_action_id_count", 4},
};

#endif