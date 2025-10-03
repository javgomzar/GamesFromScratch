#ifndef GAME_DEBUG
#define GAME_DEBUG

#include "GameStructs.h"
#include "GameRender.h"

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Debug info                                                                                                                      |
// +---------------------------------------------------------------------------------------------------------------------------------+

struct debug_entry {
    char Name[64];
    debug_entry* Parent;
    void* Value;
    debug_type Type;
    bool Editable;
    char ValueString[128];
};

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

debug_entry* _AddDebugEntry(
    debug_info* DebugInfo, 
    const char* Name, 
    debug_type Type, 
    int Size, 
    void* Value, 
    bool Editable, 
    debug_entry* Parent = NULL
) {
    debug_entry* Entry = &DebugInfo->Entries[DebugInfo->nEntries++];
    strcpy_s(Entry->Name, Name);
    Entry->Parent = Parent;
    Entry->Value = Value;
    Entry->Type = Type;
    Entry->Editable = Editable;

    return Entry;
}

debug_entry* _AddDebugArray(
    debug_info* DebugInfo,
    const char* Name,
    debug_type Type,
    int Size,
    void* Value,
    uint32 Count,
    debug_entry* Parent = NULL
) {
    uint8* Memory = (uint8*)Value;
    char Buffer[64];
    debug_entry* Result = 0;
    for (int i = 0; i < Count; i++) {
        sprintf_s(Buffer, "%s[%d]", Name, i);
        
        if (i == 0) Result = _AddDebugEntry(DebugInfo, Buffer, Type, Size, Memory, false, Parent);
        else        _AddDebugEntry(DebugInfo, Buffer, Type, Size, Memory, false, Parent);
        Memory += Size;
    }
    return Result;
}

debug_entry* _AddDebugPointerArray(
    debug_info* DebugInfo,
    const char* Name,
    debug_type Type,
    int Size,
    void* Value,
    uint32 Count,
    debug_entry* Parent = NULL
) {
    uint8* Memory = (uint8*)Value;
    char Buffer[64];
    debug_entry* Result = 0;
    for (int i = 0; i < Count; i++) {
        sprintf_s(Buffer, "%s[%d]", Name, i);
        
        if (i == 0) Result = _AddDebugEntry(DebugInfo, Buffer, Type, Size, *(void**)Memory, false, Parent);
        else        _AddDebugEntry(DebugInfo, Buffer, Type, Size, *(void**)Memory, false, Parent);
        Memory += sizeof(void*);
    }
    return Result;
}

#define DEBUG_VALUE(Variable, Type)               _AddDebugEntry(DebugInfo, #Variable, Debug_Type_##Type, sizeof(Type), &(Variable), false)
#define DEBUG_POINTER(Pointer, Type)              _AddDebugEntry(DebugInfo, #Pointer,  Debug_Type_##Type, sizeof(Type), (void*)Pointer, false)
#define DEBUG_ARRAY(Pointer, Count, Type)         _AddDebugArray(DebugInfo, #Pointer,  Debug_Type_##Type, sizeof(Type), (void*)Pointer, Count)
#define DEBUG_POINTER_ARRAY(Pointer, Count, Type) _AddDebugPointerArray(DebugInfo, #Pointer,  Debug_Type_##Type, sizeof(Type), (void*)Pointer, Count)
#define DEBUG_EDIT_VALUE(Variable, Type)          _AddDebugEntry(DebugInfo, #Variable, Debug_Type_##Type, sizeof(Type), &(Variable), true)

const float DEBUG_ENTRIES_TEXT_POINTS = 10.0f;

void UpdateAndSizeDebugEntry(game_font* Font, debug_entry* Entry, float* OutWidth, float* OutHeight) {
    float Points = DEBUG_ENTRIES_TEXT_POINTS;

    float ValueWidth = 0, ValueHeight = 0;
    if (Entry->Value) {
        switch(Entry->Type) {
            case Debug_Type_bool: {
                bool Value = *(bool*)Entry->Value;
                sprintf_s(Entry->ValueString, "%s", Value ? "true" : "false");
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_char: {
                char Value = *(char*)Entry->Value;
                switch (Value) {
                    case '\a': { strcpy_s(Entry->ValueString, "\'\\a\'"); } break;
                    case '\b': { strcpy_s(Entry->ValueString, "\'\\b\'"); } break;
                    case '\f': { strcpy_s(Entry->ValueString, "\'\\f\'"); } break;
                    case '\n': { strcpy_s(Entry->ValueString, "\'\\n\'"); } break;
                    case '\r': { strcpy_s(Entry->ValueString, "\'\\r\'"); } break;
                    case '\t': { strcpy_s(Entry->ValueString, "\'\\t\'"); } break;
                    case '\v': { strcpy_s(Entry->ValueString, "\'\\v\'"); } break;
                    case '\\': { strcpy_s(Entry->ValueString, "\'\\\\\'"); } break;
                    case '\'': { strcpy_s(Entry->ValueString, "\'\\'\'"); } break;
                    case '\"': { strcpy_s(Entry->ValueString, "\'\\\"\'"); } break;
                    case '\0': { strcpy_s(Entry->ValueString, "\'\\0\'"); } break;
                    default: {
                        if (Value >= ' ' && Value <= '~') {
                            Entry->ValueString[0] = '\'';
                            Entry->ValueString[1] = Value;
                            Entry->ValueString[2] = '\'';
                            Entry->ValueString[3] = '\0';
                        } else {
                            sprintf_s(Entry->ValueString, "\'\\x%02x\'", (unsigned char)Value);
                        }
                    }
                }
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_string: {
                sprintf_s(Entry->ValueString, "\"%s\"", (char*)Entry->Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_int8: {
                int8 Value = *(int8*)Entry->Value;
                sprintf_s(Entry->ValueString, "%d", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_int16: {
                int16 Value = *(int16*)Entry->Value;
                sprintf_s(Entry->ValueString, "%d", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_int: {
                int Value = *(int*)Entry->Value;
                sprintf_s(Entry->ValueString, "%d", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_int32:{
                int32 Value = *(int32*)Entry->Value;
                sprintf_s(Entry->ValueString, "%d", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_int64:{
                int64 Value = *(int64*)Entry->Value;
                sprintf_s(Entry->ValueString, "%I64d", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_uint8:{
                uint8 Value = *(uint8*)Entry->Value;
                sprintf_s(Entry->ValueString, "%u", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_uint16:{
                uint16 Value = *(uint16*)Entry->Value;
                sprintf_s(Entry->ValueString, "%u", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_uint32:{
                uint32 Value = *(uint32*)Entry->Value;
                sprintf_s(Entry->ValueString, "%u", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_uint64: {
                uint64 Value = *(uint64*)Entry->Value;
                sprintf_s(Entry->ValueString, "%I64u", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_memory_index: {
                memory_index Value = *(memory_index*)Entry->Value;
                sprintf_s(Entry->ValueString, "%I64u", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_float: {
                float Value = *(float*)Entry->Value;
                sprintf_s(Entry->ValueString, "%.3f", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_double: {
                double Value = *(double*)Entry->Value;
                sprintf_s(Entry->ValueString, "%.3f", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_v2: {
                v2 Value = *(v2*)Entry->Value;
                sprintf_s(Entry->ValueString, "V2(%.3f, %.3f)", Value.X, Value.Y);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_v3: {
                v3 Value = *(v3*)Entry->Value;
                sprintf_s(Entry->ValueString, "V3(%.3f, %.3f, %.3f)", Value.X, Value.Y, Value.Z);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_v4: {
                v4 Value = *(v4*)Entry->Value;
                sprintf_s(Entry->ValueString, "V4(%.3f, %.3f, %.3f, %.3f)", Value.X, Value.Y, Value.Z, Value.W);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_scale: {
                scale Value = *(scale*)Entry->Value;
                sprintf_s(Entry->ValueString, "Scale(%.3f, %.3f, %.3f)", Value.X, Value.Y, Value.Z);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_quaternion: {
                quaternion Value = *(quaternion*)Entry->Value;
                sprintf_s(Entry->ValueString, "%.3f + %.3fi + %.3fj + %.3fk", Value.c, Value.i, Value.j, Value.k);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_color: {
                Entry->ValueString[0] = '\0';
                ValueWidth = 2.0f * GetCharMaxHeight(Font, Points);
            } break;

            case Debug_Type_collider: {
                collider Value = *(collider*)Entry->Value;
                switch(Value.Type) {
                    case Rect_Collider: {
                        sprintf_s(Entry->ValueString, "(Rect) %.3f x %.3f", Value.Rect.HalfWidth, Value.Rect.HalfHeight);
                    } break;

                    case Cube_Collider: {
                        sprintf_s(Entry->ValueString, "(Cube) %.3f x %.3f x %.3f", Value.Cube.HalfWidth, Value.Cube.HalfHeight, Value.Cube.HalfDepth);
                    } break;

                    case Sphere_Collider: {
                        sprintf_s(Entry->ValueString, "(Sphere) Radius=%.3f", Value.Sphere.Radius);
                    } break;

                    case Capsule_Collider: {
                        sprintf_s(Entry->ValueString, "(Capsule) Radius=%.3f", Value.Capsule.Distance);
                    } break;

                    default: Raise("Invalid collider type");
                }

                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case Debug_Type_memory_arena: {
                memory_arena Arena = *(memory_arena*)Entry->Value;
                sprintf_s(Entry->ValueString, "%.3f", (float)Arena.Used / (float)Arena.Size);
                *OutWidth = 450.0f;
                *OutHeight = 20.0f;
                return;
            } break;
            
            default: {
                if (IsEnumType(Entry->Type)) {
                    int Value = *(int*)Entry->Value;
                    for (int i = 0; i < ENUM_VALUES_SIZE; i++) {
                        debug_enum_value EnumValue = EnumValues[i];
                        if (EnumValue.EnumType == Entry->Type && EnumValue.Value == Value) {
                            sprintf_s(Entry->ValueString, "%s (%d)", EnumValue.Identifier, Value);
                            GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                            break;
                        }
                    }
                }
                else if (IsFlagType(Entry->Type)) {
                    int Matches = 0;
                    int Value = *(int*)Entry->Value;
                    for (int i = 0; i < FLAG_VALUES_SIZE; i++) {
                        debug_enum_value FlagValue = FlagValues[i];
                        if (FlagValue.EnumType == Entry->Type && (FlagValue.Value & Value)) {
                            if (Matches == 0) {
                                sprintf_s(Entry->ValueString, "%s", FlagValue.Identifier);
                            }
                            else {
                                strcat_s(Entry->ValueString, " | ");
                                strcat_s(Entry->ValueString, FlagValue.Identifier);
                            }

                            Matches++;
                        }
                    }
                    char Buffer[16];
                    sprintf_s(Buffer, " (%d)", Value);
                    strcat_s(Entry->ValueString, Buffer);
                    GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                }
                else if (IsStructType(Entry->Type)) {
                    if (Entry->Value == 0) {
                        sprintf_s(Entry->ValueString, "NULL");
                        GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                    }
                    else {
                        sprintf_s(Entry->ValueString, "0x%016llx", (uint64)Entry->Value);
                        GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                    }
                }
                else Raise("Invalid debug type.");
            }
        }
    }
    else {
        sprintf_s(Entry->ValueString, "NULL");
        GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
    }

    float Width = 0,     Height = GetCharMaxHeight(Font, Points), 
      NameWidth = 0, NameHeight = 0;
    GetTextWidthAndHeight(Entry->Name, Font, Points, &Width, &Height);
    GetTextWidthAndHeight(": ", Font, Points, &NameWidth, &NameHeight);
    Width += NameWidth + ValueWidth;
    Height = max(Height, ValueHeight);

    debug_entry* Parent = Entry->Parent;
    while (Parent) {
        Width += 20.0f;
        Parent = Parent->Parent;
    }

    *OutWidth = Width;
    *OutHeight = Height;
}

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
            rectangle Rect = { TextCursor.X, Position.Y + 3.0f, 2.0f * LineHeight, LineHeight };
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
            if (IsEnumType(Entry->Type) || IsFlagType(Entry->Type) || IsStructType(Entry->Type)) {
                PushText(Group, TextCursor, Font_Menlo_Regular_ID, Entry->ValueString, White, Points);
            }
        }
    }
}

#endif