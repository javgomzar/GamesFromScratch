#include "pch.h"
#include "enum.h"


bool IsEnumType(Type type) { return type > 23 && type < 32; }
bool IsFlagType(Type type) { return type > 31 && type < 36; }

const Debug_Enum_Value ENUM_VALUES[ENUM_VALUES_SIZE] = {
    {Type_Topology, "Topology_Point", 0},
    {Type_Topology, "Topology_Line", 1},
    {Type_Topology, "Topology_Line_Strip", 2},
    {Type_Topology, "Topology_Triangle", 3},
    {Type_Topology, "Topology_Triangle_Strip", 4},
    {Type_Topology, "Topology_Patches", 5},
    {Type_Topology, "Topology_count", 6},
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
};

int _RandomEnum(Type type) {
    for (int i = 0; i < ENUM_VALUES_SIZE; ++i) {
        Debug_Enum_Value entry = ENUM_VALUES[i];
        if (entry.enum_type == type) {
            return RNG::RandInt(0, entry.value);
        }
    }
    Log::Error("Type wasn't found in enum list.");
    return 0;
}

const Debug_Enum_Value FLAG_VALUES[FLAG_VALUES_SIZE] = {
    {Type_Entity_Flag, "Render", 1},
    {Type_Entity_Flag, "Movable", 2},
    {Type_Entity_Flag, "Animated", 4},
    {Type_UI_Flags, "Drag", 1},
    {Type_UI_Draw, "Rect", 1},
    {Type_UI_Draw, "Text", 2},
    {Type_UI_Draw, "Fillbar", 4},
    {Type_UI_Draw, "Slider", 8},
    {Type_UI_Draw, "Checkbox", 16},
    {Type_Render_Flag, "depth_test", 1},
    {Type_Render_Flag, "stencil_test", 2},
    {Type_Render_Flag, "overwrite_alpha", 4},
    {Type_Render_Flag, "text_outline", 8},
    {Type_Render_Flag, "debug_bones", 16},
    {Type_Render_Flag, "sky", 32},
    {Type_Render_Flag, "water", 64},
};


Entity_Flag operator|(Entity_Flag lhs, Entity_Flag rhs) { return (Entity_Flag)((uint64)lhs | (uint64)rhs); }
Entity_Flag& operator|=(Entity_Flag& lhs, Entity_Flag rhs) { return lhs = lhs | rhs; }
bool operator&(Entity_Flag lhs, Entity_Flag rhs) { return (uint64)lhs & (uint64)rhs; }
Entity_Flag operator~(Entity_Flag flag) { return (Entity_Flag)(~(uint64)flag); }

UI_Flags operator|(UI_Flags lhs, UI_Flags rhs) { return (UI_Flags)((uint64)lhs | (uint64)rhs); }
UI_Flags& operator|=(UI_Flags& lhs, UI_Flags rhs) { return lhs = lhs | rhs; }
bool operator&(UI_Flags lhs, UI_Flags rhs) { return (uint64)lhs & (uint64)rhs; }
UI_Flags operator~(UI_Flags flag) { return (UI_Flags)(~(uint64)flag); }

UI_Draw operator|(UI_Draw lhs, UI_Draw rhs) { return (UI_Draw)((uint64)lhs | (uint64)rhs); }
UI_Draw& operator|=(UI_Draw& lhs, UI_Draw rhs) { return lhs = lhs | rhs; }
bool operator&(UI_Draw lhs, UI_Draw rhs) { return (uint64)lhs & (uint64)rhs; }
UI_Draw operator~(UI_Draw flag) { return (UI_Draw)(~(uint64)flag); }

Render_Flag operator|(Render_Flag lhs, Render_Flag rhs) { return (Render_Flag)((uint64)lhs | (uint64)rhs); }
Render_Flag& operator|=(Render_Flag& lhs, Render_Flag rhs) { return lhs = lhs | rhs; }
bool operator&(Render_Flag lhs, Render_Flag rhs) { return (uint64)lhs & (uint64)rhs; }
Render_Flag operator~(Render_Flag flag) { return (Render_Flag)(~(uint64)flag); }
