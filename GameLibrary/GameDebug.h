#ifndef GAME_DEBUG
#define GAME_DEBUG

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Debug values                                                                                                                    |
// +---------------------------------------------------------------------------------------------------------------------------------+

#include "GameDebugTypes.h"
#include "GameRender.h"

struct debug_entry {
    char Name[64];
    debug_entry* Parent;
    void* Value;
    debug_type Type;
    bool Editable;
    char ValueString[64];
};

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Debug info                                                                                                                      |
// +---------------------------------------------------------------------------------------------------------------------------------+

const int MAX_DEBUG_ENTRIES = 128;
struct debug_info {
    debug_entry Entries[MAX_DEBUG_ENTRIES];
    int nEntries;
    float FPS;
    float BudgetTime;
    float UsedTime;
    float UsedMCyclesPerFrame;
    float UsedMemory;
};

debug_entry* _AddDebugEntry(debug_info* DebugInfo, char* Name, debug_type Type, int Size, void* Value, bool Editable, debug_entry* Parent = 0) {
    debug_entry* Entry = &DebugInfo->Entries[DebugInfo->nEntries++];
    strcpy_s(Entry->Name, Name);
    Entry->Parent = Parent;
    Entry->Value = Value;
    Entry->Type = Type;
    Entry->Editable = Editable;

    return Entry;
}

debug_entry* _AddDebugArray(debug_info* DebugInfo, char* Name, debug_type Type, int Size, void* Value, uint32 Count) {
    uint8* Memory = (uint8*)Value;
    char Buffer[64];
    debug_entry* Result = 0;
    for (int i = 0; i < Count; i++) {
        sprintf_s(Buffer, "%s[%d]", Name, i);
        
        if (i == 0) Result = _AddDebugEntry(DebugInfo, Buffer, Type, Size, Memory, false);
        else        _AddDebugEntry(DebugInfo, Buffer, Type, Size, Memory, false);
        Memory += Size;
    }
    return Result;
}

#define DEBUG_ARRAY(Pointer, Count, Type) _AddDebugArray(DebugInfo, #Pointer,  Debug_Type_##Type, sizeof(Type), (void*)Pointer, Count)
#define DEBUG_POINTER(Pointer, Type)      _AddDebugEntry(DebugInfo, #Pointer,  Debug_Type_##Type, sizeof(Type), (void*)Pointer, false)
#define DEBUG_VALUE(Variable, Type)       _AddDebugEntry(DebugInfo, #Variable, Debug_Type_##Type, sizeof(Type), &(Variable), false)
#define DEBUG_EDIT_VALUE(Variable, Type)  _AddDebugEntry(DebugInfo, #Variable, Debug_Type_##Type, sizeof(Type), &(Variable), true)

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Debug                                                                                                                                                            |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

const float DEBUG_ENTRIES_TEXT_POINTS = 10.0f;

void PushDebugEntry(render_group* Group, debug_entry* Entry, v2 Position, color Color) {
    TIMED_BLOCK;
    
    game_font* Font = GetAsset(Group->Assets, Font_Menlo_Regular_ID);
    char Buffer[128];
    float Points = DEBUG_ENTRIES_TEXT_POINTS;
    float LineHeight = GetCharMaxHeight(Font, Points);

    v2 TextCursor = Position + V2(0, LineHeight);
    debug_entry* Parent = Entry->Parent;
    while (Parent) {
        TextCursor.X += 20.0f;
        Parent = Parent->Parent;
    }

    sprintf_s(Buffer, "%s: ", Entry->Name);
    if (Entry->Type != Debug_Type_memory_arena) {
        PushText(Group, TextCursor, Font_Menlo_Regular_ID, Buffer, Color, Points);
    }

    float Width, Height;
    GetTextWidthAndHeight(Buffer, Font, Points, &Width, &Height);
    TextCursor.X += Width;

    switch(Entry->Type) {
        case Debug_Type_bool: {
            bool Value = *(bool*)Entry->Value;
            PushText(Group, TextCursor, Font_Menlo_Regular_ID, Entry->ValueString, Value? Cyan : Red, Points);
        } break;

        case Debug_Type_char:
        case Debug_Type_string:
        case Debug_Type_int8:
        case Debug_Type_int16:
        case Debug_Type_int:
        case Debug_Type_int32:
        case Debug_Type_int64:
        case Debug_Type_uint8:
        case Debug_Type_uint16:
        case Debug_Type_uint32:
        case Debug_Type_uint64:
        case Debug_Type_memory_index:
        case Debug_Type_float:
        case Debug_Type_double:
        case Debug_Type_v2:
        case Debug_Type_v3:
        case Debug_Type_v4:
        case Debug_Type_scale:
        case Debug_Type_quaternion:
        case Debug_Type_collider:
        {
            PushText(Group, TextCursor, Font_Menlo_Regular_ID, Entry->ValueString, White, Points);
        } break;

        case Debug_Type_color: {
            rectangle Rect = { TextCursor.X, Position.Y, 2.0f * LineHeight, LineHeight };
            color Color = *(color*)Entry->Value;
            PushRect(Group, Rect, Color);
            PushRectOutline(Group, Rect, Gray, 1.0f);
        } break;

        case Debug_Type_memory_arena: {
            tokenizer Tokenizer = InitTokenizer(Entry->ValueString);
            float Percentage = ParseFloat(Tokenizer);
            rectangle Rect = Rectangle(Position.X, Position.Y, 450.0f, 20.0f);
            PushFillbar(Group, Entry->Name, Percentage, Rect);
        } break;

        default: {
            if (IsEnumType(Entry->Type)) {
                PushText(Group, TextCursor, Font_Menlo_Regular_ID, Entry->ValueString, White, Points);
            }
            else if (IsStructType(Entry->Type)) {
                PushText(Group, TextCursor, Font_Menlo_Regular_ID, Entry->ValueString, White, Points);
            }
        }
    }
}

void PushDebugVector(render_group* Group, v2 Vector, v2 Position, color Color) {
    v2 Orthogonal = perp(normalize(V2(Vector.X, Vector.Y)));
    float OrthogonalLength = 0.005333f * Group->Height;

    int Thickness = max(1.0f, 0.0025f * Group->Height);
    PushLine(Group, Position, Position + 0.875 * Vector, Color, Thickness, SORT_ORDER_DEBUG_OVERLAY);
    triangle2 Arrowhead1 = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector - OrthogonalLength * Orthogonal
    };
    PushTriangle(
        Group,
        Arrowhead1,
        Color, 
        SORT_ORDER_DEBUG_OVERLAY
    );
    triangle2 Arrowhead2 = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector + OrthogonalLength * Orthogonal,
    };
    PushTriangle(
        Group,
        Arrowhead2,
        Color, 
        SORT_ORDER_DEBUG_OVERLAY
    );
}

void PushDebugVector(render_group* Group, v3 Vector, v3 Position, color Color) {
    float Height = Group->Height;

    v2 CameraCoordinates = perp(V2(dot(Vector, Group->Camera->Basis.X), dot(Vector, Group->Camera->Basis.Y)));
    v3 Orthogonal = normalize(CameraCoordinates.X * Group->Camera->Basis.X + CameraCoordinates.Y * Group->Camera->Basis.Y);
    float OrthogonalLength = (modulus(Vector) / 15.0f);

    int Thickness = max(1.0, 0.0025 * Height);
    PushLine(Group, Position, Position + 0.875 * Vector, Color, Thickness, SORT_ORDER_MESHES);
    triangle3 Triangle = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector - OrthogonalLength * Orthogonal,
    };
    PushTriangle(Group, Triangle, Color, SORT_ORDER_MESHES);
    Triangle = {
        Position + Vector,
        Position + 0.875 * Vector,
        Position + 0.8 * Vector + OrthogonalLength * Orthogonal,
    };
    PushTriangle(Group, Triangle, Color, SORT_ORDER_MESHES);
}

void PushDebugFustrum(
    render_group* Group,
    v3 Position,
    double Angle, double Pitch,
    double l, double r, double b, double t, double n, double f
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Flags = DEPTH_TEST_RENDER_FLAG;
    render_primitive_command* Result = PushPrimitiveCommand(
        Group,
        render_primitive_line,
        White,
        Shader,
        vertex_layout_vec3_vec2_id,
        9,
        24,
        SORT_ORDER_DEBUG_OVERLAY,
        Options
    );

    basis B = GetCameraBasis(Angle, Pitch);

    v3 nv = -n * B.Z;
    v3 rv = r * B.X;
    v3 lv = -l * B.X;
    v3 tv = t * B.Y * ((double)Group->Height / (double)Group->Width);
    v3 bv = -b * B.Y * ((double)Group->Height / (double)Group->Width);
    v3 fv = -f * B.Z;

    v3 l_ = f * lv;
    v3 r_ = f * rv;
    v3 t_ = f * tv;
    v3 b_ = f * bv;

    v3* Vertices = (v3*)Result->Vertices;
    Vertices[0] = Position;
    Vertices[1] = Position + l_ + t_ + fv;
    Vertices[2] = Position + r_ + t_ + fv;
    Vertices[3] = Position + l_ + b_ + fv;
    Vertices[4] = Position + r_ + b_ + fv;
    Vertices[5] = Position + rv + bv + nv;
    Vertices[6] = Position + rv + tv + nv;
    Vertices[7] = Position + lv + bv + nv;
    Vertices[8] = Position + lv + tv + nv;

    uint32* Elements = Result->ElementEntry.Pointer;
    Elements[0]  = 0;
    Elements[1]  = 1;
    Elements[2]  = 0;
    Elements[3]  = 2;
    Elements[4]  = 0;
    Elements[5]  = 3;
    Elements[6]  = 0;
    Elements[7]  = 4;
    Elements[8]  = 4;
    Elements[9]  = 3;
    Elements[10] = 2;
    Elements[11] = 1;
    Elements[12] = 4;
    Elements[13] = 2;
    Elements[14] = 3;
    Elements[15] = 1;
    Elements[16] = 5;
    Elements[17] = 7;
    Elements[18] = 6;
    Elements[19] = 8;
    Elements[20] = 5;
    Elements[21] = 6;
    Elements[22] = 7;
    Elements[23] = 8;
}

void PushDebugGrid(render_group* Group, float Alpha) {
    const int nVertices = 404;
    
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_World_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Thickness = 1.0f;
    Options.Flags = DEPTH_TEST_RENDER_FLAG;
    v3* Vertices = (v3*)PushPrimitiveCommand(
        Group,
        render_primitive_line,
        ChangeAlpha(White, 0.5f),
        Shader,
        vertex_layout_vec3_id,
        nVertices,
        0,
        SORT_ORDER_DEBUG_OVERLAY-2.0f,
        Options
    )->Vertices;

    for (int i = 0; i <= 100; i++) {
        Vertices[4*i  ] = V3(50-i, 0, -50);
        Vertices[4*i+1] = V3(50-i, 0, 50);
        Vertices[4*i+2] = V3(-50, 0, 50-i);
        Vertices[4*i+3] = V3(50, 0, 50-i);
    }
}

void PushDebugFramebuffer(render_group* Group, render_group_target Framebuffer, bool Attachment = false) {
    render_command Command;
    Command.Index = Group->nTargets;
    Command.Priority = SORT_ORDER_PUSH_RENDER_TARGETS - 0.1f;
    Command.Type = render_target;

    PushCommand(Group, Command);

    render_target_command TargetCommand = {};
    TargetCommand.Source = Framebuffer;
    TargetCommand.Target = Target_Output;
    TargetCommand.DebugAttachment = Attachment;
    
    if (Framebuffer == Target_World || Framebuffer == Target_Outline) {
        TargetCommand.Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Antialiasing_ID);
    }
    else {
        TargetCommand.Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Framebuffer_ID);
    }

    TargetCommand.VertexEntry = PushVertexEntry(&Group->VertexBuffer, 6, vertex_layout_vec3_vec2_id);

    float* Vertices = (float*)TargetCommand.VertexEntry.Pointer;
    Vertices[0]  = 0.5f; Vertices[1]  = -1.0f; Vertices[2]  = 0.0f; Vertices[3]  = 0.0f; Vertices[4]  = 0.0f,
    Vertices[5]  = 1.0f; Vertices[6]  = -1.0f; Vertices[7]  = 0.0f; Vertices[8]  = 1.0f; Vertices[9]  = 0.0f;
    Vertices[10] = 1.0f; Vertices[11] = -0.5f; Vertices[12] = 0.0f; Vertices[13] = 1.0f; Vertices[14] = 1.0f;
    Vertices[15] = 0.5f; Vertices[16] = -1.0f; Vertices[17] = 0.0f; Vertices[18] = 0.0f; Vertices[19] = 0.0f;
    Vertices[20] = 1.0f; Vertices[21] = -0.5f; Vertices[22] = 0.0f; Vertices[23] = 1.0f; Vertices[24] = 1.0f;
    Vertices[25] = 0.5f; Vertices[26] = -0.5f; Vertices[27] = 0.0f; Vertices[28] = 0.0f; Vertices[29] = 1.0f;

    Group->TargetCommands[Group->nTargets++] = TargetCommand;
}

void PushDebugPlot(
    render_group* Group,
    int N,
    float* Data,
    v2 Position,
    int dx,
    color Color = White,
    float Thickness = 2.0f,
    float Order = SORT_ORDER_DEBUG_OVERLAY
) {
    game_shader_pipeline* Shader = GetShaderPipeline(Group->Assets, Shader_Pipeline_Screen_Single_Color_ID);
    render_primitive_options Options = {};
    Options.Thickness = Thickness;
    v2* Vertices = (v2*)PushPrimitiveCommand(
        Group,
        render_primitive_line_strip,
        Color,
        Shader,
        vertex_layout_vec2_id,
        N,
        0,
        Order,
        Options
    )->Vertices;

    float X = 0;
    for (int i = 0; i < N; i++) {
        Vertices[N] = Position + V2(X, -Data[i]);
        X += dx;
    }
}

#endif