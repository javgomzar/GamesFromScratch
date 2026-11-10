#ifndef GAME_ENUMS
#define GAME_ENUMS

enum Type {
    Type_void,
    Type_bool,
    Type_char,
    Type_String,
    Type_int8,
    Type_int16,
    Type_int,
    Type_int32,
    Type_int64,
    Type_uint8,
    Type_uint16,
    Type_uint32,
    Type_uint64,
    Type_memory_size,
    Type_float,
    Type_double,
    Type_V2,
    Type_V3,
    Type_V4,
    Type_Scale,
    Type_Quaternion,
    Type_Color,
    Type_Collider,
    Type_Arena,
    Type_Topology,
    Type_Sort_Key,
    Type_Render_Group_Target,
    Type_Color_Format,
    Type_Vertex_Type,
    Type_Vertex_ID,
    Type_Entity_type,
    Type_character_action_id,
    Type_Render_Flag,
    Type_Entity_Flag,
    Type_UI_Flags,
    Type_UI_Draw,
};

enum class Render_Flag : uint64 {
    none = 0,
    depth_test = 1 << 0,
    stencil_test = 1 << 1,
    overwrite_alpha = 1 << 2,
    text_outline = 1 << 3,
    debug_bones = 1 << 4,
    sky = 1 << 5,
    water = 1 << 6,
};

enum class Entity_Flag : uint64 {
    none = 0,
    Render = 1 << 0,
    Movable = 1 << 1,
    Animated = 1 << 2,
};

enum class UI_Flags : uint64 {
    none = 0,
    Drag = 1 << 0,
};

enum class UI_Draw : uint64 {
    none = 0,
    Rect = 1 << 0,
    Text = 1 << 1,
    Fillbar = 1 << 2,
    Slider = 1 << 3,
    Checkbox = 1 << 4,
};

bool IsEnumType(Type type) { return type > 23 && type < 32; }
bool IsFlagType(Type type) { return type > 31 && type < 36; }

struct Debug_Enum_Value {
    Type enum_type;
    const char* identifier;
    int value;
};

const int ENUM_VALUES_SIZE = 55;
Debug_Enum_Value ENUM_VALUES[ENUM_VALUES_SIZE] = {
    {Type_Topology, "Topology_Point", 0},
    {Type_Topology, "Topology_Line", 1},
    {Type_Topology, "Topology_Line_Strip", 2},
    {Type_Topology, "Topology_Triangle", 3},
    {Type_Topology, "Topology_Triangle_Strip", 4},
    {Type_Topology, "Topology_Patches", 5},
    {Type_Topology, "Topology_count", 6},
    {Type_Sort_Key, "Sort_Background", 0},
    {Type_Sort_Key, "Sort_Terrain", 1},
    {Type_Sort_Key, "Sort_Meshes", 2},
    {Type_Sort_Key, "Sort_UI", 3},
    {Type_Sort_Key, "Sort_Debug", 4},
    {Type_Sort_Key, "Sort_Key_count", 5},
    {Type_Render_Group_Target, "Target_None", 0},
    {Type_Render_Group_Target, "Target_World", 1},
    {Type_Render_Group_Target, "Target_Outline", 2},
    {Type_Render_Group_Target, "Target_Postprocessing_Outline", 3},
    {Type_Render_Group_Target, "Target_PingPong", 4},
    {Type_Render_Group_Target, "Target_Fluid", 5},
    {Type_Render_Group_Target, "Target_Output", 6},
    {Type_Render_Group_Target, "Render_Group_Target_count", 7},
    {Type_Color_Format, "Color_Format_R", 0},
    {Type_Color_Format, "Color_Format_RG", 1},
    {Type_Color_Format, "Color_Format_RGB", 2},
    {Type_Color_Format, "Color_Format_RGBA", 3},
    {Type_Color_Format, "Color_Format_count", 4},
    {Type_Vertex_Type, "Vertex_Type_empty", 0},
    {Type_Vertex_Type, "Vertex_Type_float", 1},
    {Type_Vertex_Type, "Vertex_Type_V2", 2},
    {Type_Vertex_Type, "Vertex_Type_V3", 3},
    {Type_Vertex_Type, "Vertex_Type_V4", 4},
    {Type_Vertex_Type, "Vertex_Type_int", 5},
    {Type_Vertex_Type, "Vertex_Type_IV2", 6},
    {Type_Vertex_Type, "Vertex_Type_IV3", 7},
    {Type_Vertex_Type, "Vertex_Type_IV4", 8},
    {Type_Vertex_Type, "Vertex_Type_Matrix2", 9},
    {Type_Vertex_Type, "Vertex_Type_Matrix3", 10},
    {Type_Vertex_Type, "Vertex_Type_Matrix4", 11},
    {Type_Vertex_Type, "Vertex_Type_count", 12},
    {Type_Vertex_ID, "Vertex_Position_ID", 0},
    {Type_Vertex_ID, "Vertex_Joint_ID", 1},
    {Type_Vertex_ID, "Vertex_Surface_ID", 2},
    {Type_Vertex_ID, "Vertex_Text_ID", 3},
    {Type_Vertex_ID, "Vertex_ID_count", 4},
    {Type_Entity_type, "Character_Entity_Type", 0},
    {Type_Entity_type, "Enemy_Entity_Type", 1},
    {Type_Entity_type, "Camera_Entity_Type", 2},
    {Type_Entity_type, "Prop_Entity_Type", 3},
    {Type_Entity_type, "Weapon_Entity_Type", 4},
    {Type_Entity_type, "Entity_type_count", 5},
    {Type_character_action_id, "Character_Action_Idle_ID", 0},
    {Type_character_action_id, "Character_Action_Walk_ID", 1},
    {Type_character_action_id, "Character_Action_Jump_ID", 2},
    {Type_character_action_id, "Character_Action_Attack_ID", 3},
    {Type_character_action_id, "character_action_id_count", 4},
};

const int FLAG_VALUES_SIZE = 16;
Debug_Enum_Value FLAG_VALUES[FLAG_VALUES_SIZE] = {
    {Type_Render_Flag, "depth_test", 1},
    {Type_Render_Flag, "stencil_test", 2},
    {Type_Render_Flag, "overwrite_alpha", 4},
    {Type_Render_Flag, "text_outline", 8},
    {Type_Render_Flag, "debug_bones", 16},
    {Type_Render_Flag, "sky", 32},
    {Type_Render_Flag, "water", 64},
    {Type_Entity_Flag, "Render", 1},
    {Type_Entity_Flag, "Movable", 2},
    {Type_Entity_Flag, "Animated", 4},
    {Type_UI_Flags, "Drag", 1},
    {Type_UI_Draw, "Rect", 1},
    {Type_UI_Draw, "Text", 2},
    {Type_UI_Draw, "Fillbar", 4},
    {Type_UI_Draw, "Slider", 8},
    {Type_UI_Draw, "Checkbox", 16},
};

constexpr Render_Flag operator|(Render_Flag a, Render_Flag b) { return (Render_Flag)((uint64)a | (uint64)b); }
constexpr Render_Flag& operator|=(Render_Flag& a, Render_Flag b) { return a = a | b; }
constexpr bool operator&(Render_Flag a, Render_Flag b) { return (uint64)a & (uint64)b; }
constexpr Render_Flag operator~(Render_Flag a) { return (Render_Flag)(~(uint64)a); }

constexpr Entity_Flag operator|(Entity_Flag a, Entity_Flag b) { return (Entity_Flag)((uint64)a | (uint64)b); }
constexpr Entity_Flag& operator|=(Entity_Flag& a, Entity_Flag b) { return a = a | b; }
constexpr bool operator&(Entity_Flag a, Entity_Flag b) { return (uint64)a & (uint64)b; }
constexpr Entity_Flag operator~(Entity_Flag a) { return (Entity_Flag)(~(uint64)a); }

constexpr UI_Flags operator|(UI_Flags a, UI_Flags b) { return (UI_Flags)((uint64)a | (uint64)b); }
constexpr UI_Flags& operator|=(UI_Flags& a, UI_Flags b) { return a = a | b; }
constexpr bool operator&(UI_Flags a, UI_Flags b) { return (uint64)a & (uint64)b; }
constexpr UI_Flags operator~(UI_Flags a) { return (UI_Flags)(~(uint64)a); }

constexpr UI_Draw operator|(UI_Draw a, UI_Draw b) { return (UI_Draw)((uint64)a | (uint64)b); }
constexpr UI_Draw& operator|=(UI_Draw& a, UI_Draw b) { return a = a | b; }
constexpr bool operator&(UI_Draw a, UI_Draw b) { return (uint64)a & (uint64)b; }
constexpr UI_Draw operator~(UI_Draw a) { return (UI_Draw)(~(uint64)a); }

#endif