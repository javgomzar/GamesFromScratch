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
    Debug_Type_render_command_type,
    Debug_Type_render_group_target,
    Debug_Type_render_primitive,
    Debug_Type_color_format,
    Debug_Type_shader_pass_type,
    Debug_Type_compute_type,
    Debug_Type_render_flags,
    Debug_Type_transform,
    Debug_Type_game_entity,
};

enum render_flags {
    DEPTH_TEST_FLAG = 1 << 0,
    STENCIL_TEST_FLAG = 1 << 1,
    OVERWRITE_ALPHA_FLAG = 1 << 2,
    TEXT_OUTLINE_FLAG = 1 << 3,
    DEBUG_BONES_FLAG = 1 << 4,
    SKY_FLAG = 1 << 5,
    SKY_DEPTH_FLAG = 1 << 6,
    STAR_FLAG = 1 << 7,
    WATER_FLAG = 1 << 8,
};

bool IsEnumType(debug_type Type) { return Type > 22 && Type < 34; }
bool IsFlagType(debug_type Type) { return Type > 33 && Type < 35; }

struct debug_enum_value {
    debug_type EnumType;
    const char* Identifier;
    int Value;
};

const int ENUM_VALUES_SIZE = 57;
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
    {Debug_Type_game_text_id, "Text_Constellations_ID", 0},
    {Debug_Type_game_text_id, "Text_Stars_ID", 1},
    {Debug_Type_game_text_id, "game_text_id_count", 2},
    {Debug_Type_game_heightmap_id, "Heightmap_Spain_ID", 0},
    {Debug_Type_game_heightmap_id, "game_heightmap_id_count", 1},
    {Debug_Type_game_animation_id, "Animation_Idle_ID", 0},
    {Debug_Type_game_animation_id, "Animation_Walk_ID", 1},
    {Debug_Type_game_animation_id, "Animation_Jump_ID", 2},
    {Debug_Type_game_animation_id, "Animation_Attack_ID", 3},
    {Debug_Type_game_animation_id, "game_animation_id_count", 4},
    {Debug_Type_game_entity_type, "Entity_Type_Camera", 0},
    {Debug_Type_game_entity_type, "game_entity_type_count", 1},
    {Debug_Type_render_command_type, "render_clear", 0},
    {Debug_Type_render_command_type, "render_draw_primitive", 1},
    {Debug_Type_render_command_type, "render_text", 2},
    {Debug_Type_render_command_type, "render_mesh", 3},
    {Debug_Type_render_command_type, "render_shader_pass", 4},
    {Debug_Type_render_command_type, "render_compute", 5},
    {Debug_Type_render_command_type, "render_target", 6},
    {Debug_Type_render_command_type, "render_command_type_count", 7},
    {Debug_Type_render_group_target, "Target_None", 0},
    {Debug_Type_render_group_target, "Target_World", 1},
    {Debug_Type_render_group_target, "Target_Outline", 2},
    {Debug_Type_render_group_target, "Target_Postprocessing_Outline", 3},
    {Debug_Type_render_group_target, "Target_PingPong", 4},
    {Debug_Type_render_group_target, "Target_Fluid", 5},
    {Debug_Type_render_group_target, "Target_Output", 6},
    {Debug_Type_render_group_target, "render_group_target_count", 7},
    {Debug_Type_render_primitive, "render_primitive_point", 0},
    {Debug_Type_render_primitive, "render_primitive_line", 1},
    {Debug_Type_render_primitive, "render_primitive_line_strip", 2},
    {Debug_Type_render_primitive, "render_primitive_triangle", 3},
    {Debug_Type_render_primitive, "render_primitive_triangle_strip", 4},
    {Debug_Type_render_primitive, "render_primitive_patches", 5},
    {Debug_Type_render_primitive, "render_primitive_count", 6},
    {Debug_Type_color_format, "Color_Format_R", 0},
    {Debug_Type_color_format, "Color_Format_RG", 1},
    {Debug_Type_color_format, "Color_Format_RGB", 2},
    {Debug_Type_color_format, "Color_Format_RGBA", 3},
    {Debug_Type_color_format, "color_format_count", 4},
    {Debug_Type_shader_pass_type, "shader_pass_empty", 0},
    {Debug_Type_shader_pass_type, "shader_pass_type_count", 1},
    {Debug_Type_compute_type, "compute_kernel", 0},
    {Debug_Type_compute_type, "compute_outline_init", 1},
    {Debug_Type_compute_type, "compute_jump_flood", 2},
    {Debug_Type_compute_type, "compute_outline", 3},
    {Debug_Type_compute_type, "compute_fft", 4},
    {Debug_Type_compute_type, "compute_type_count", 5},
};

const int FLAG_VALUES_SIZE = 9;
debug_enum_value FlagValues[FLAG_VALUES_SIZE] = {
    {Debug_Type_render_flags, "DEPTH_TEST_FLAG", 1},
    {Debug_Type_render_flags, "STENCIL_TEST_FLAG", 2},
    {Debug_Type_render_flags, "OVERWRITE_ALPHA_FLAG", 4},
    {Debug_Type_render_flags, "TEXT_OUTLINE_FLAG", 8},
    {Debug_Type_render_flags, "DEBUG_BONES_FLAG", 16},
    {Debug_Type_render_flags, "SKY_FLAG", 32},
    {Debug_Type_render_flags, "SKY_DEPTH_FLAG", 64},
    {Debug_Type_render_flags, "STAR_FLAG", 128},
    {Debug_Type_render_flags, "WATER_FLAG", 256},
};

#endif