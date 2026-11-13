#pragma once

#include <platform.h>
#include <mathematics.h>


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
    Type_Entity_type,
    Type_character_action_id,
    Type_Sort_Key,
    Type_Render_Group_Target,
    Type_Color_Format,
    Type_Vertex_Type,
    Type_Vertex_ID,
    Type_Entity_Flag,
    Type_UI_Flags,
    Type_UI_Draw,
    Type_Render_Flag,
    Type_Transform,
};

enum class Entity_Flag : uint64 {
    None = 0,
    Render = 1 << 0,
    Movable = 1 << 1,
    Animated = 1 << 2,
};

enum class UI_Flags : uint64 {
    None = 0,
    Drag = 1 << 0,
};

enum class UI_Draw : uint64 {
    None = 0,
    Rect = 1 << 0,
    Text = 1 << 1,
    Fillbar = 1 << 2,
    Slider = 1 << 3,
    Checkbox = 1 << 4,
};

enum class Render_Flag : uint64 {
    None = 0,
    depth_test = 1 << 0,
    stencil_test = 1 << 1,
    overwrite_alpha = 1 << 2,
    text_outline = 1 << 3,
    debug_bones = 1 << 4,
    sky = 1 << 5,
    water = 1 << 6,
};

bool IsEnumType(Type type);
bool IsFlagType(Type type);

struct Debug_Enum_Value {
    Type enum_type;
    const char* identifier;
    int value;
};

inline constexpr int ENUM_VALUES_SIZE = 55;
extern const Debug_Enum_Value ENUM_VALUES[ENUM_VALUES_SIZE];
int _RandomEnum(Type type);
#define RandomEnum(type) (type)_RandomEnum(Type_##type)

inline constexpr int FLAG_VALUES_SIZE = 16;
extern const Debug_Enum_Value FLAG_VALUES[FLAG_VALUES_SIZE];

Entity_Flag operator|(Entity_Flag lhs, Entity_Flag rhs);
Entity_Flag& operator|=(Entity_Flag& lhs, Entity_Flag rhs);
bool operator&(Entity_Flag lhs, Entity_Flag rhs);
Entity_Flag operator~(Entity_Flag flag);

UI_Flags operator|(UI_Flags lhs, UI_Flags rhs);
UI_Flags& operator|=(UI_Flags& lhs, UI_Flags rhs);
bool operator&(UI_Flags lhs, UI_Flags rhs);
UI_Flags operator~(UI_Flags flag);

UI_Draw operator|(UI_Draw lhs, UI_Draw rhs);
UI_Draw& operator|=(UI_Draw& lhs, UI_Draw rhs);
bool operator&(UI_Draw lhs, UI_Draw rhs);
UI_Draw operator~(UI_Draw flag);

Render_Flag operator|(Render_Flag lhs, Render_Flag rhs);
Render_Flag& operator|=(Render_Flag& lhs, Render_Flag rhs);
bool operator&(Render_Flag lhs, Render_Flag rhs);
Render_Flag operator~(Render_Flag flag);
