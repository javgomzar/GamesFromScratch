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
    for (int i = 0; i < strlen(Name); i++) Entry->Name[i] = Name[i];
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
    debug_entry* Result = 0;
    for (int i = 0; i < Count; i++) {
        std::string Text = std::format("{}[{}]", Name, i);
        if (i == 0) Result = _AddDebugEntry(DebugInfo, Text.c_str(), Type, Size, Memory, false, Parent);
        else        _AddDebugEntry(DebugInfo, Text.c_str(), Type, Size, Memory, false, Parent);
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
    debug_entry* Result = 0;
    for (int i = 0; i < Count; i++) {
        std::string Text = std::format("{}[{}]", Name, i);
        if (i == 0) Result = _AddDebugEntry(DebugInfo, Text.c_str(), Type, Size, *(void**)Memory, false, Parent);
        else        _AddDebugEntry(DebugInfo, Text.c_str(), Type, Size, *(void**)Memory, false, Parent);
        Memory += sizeof(void*);
    }
    return Result;
}

#define DEBUG_VALUE(Variable, Type)               _AddDebugEntry(DebugInfo, #Variable, debug_##Type, sizeof(Type), &(Variable), false)
#define DEBUG_POINTER(Pointer, Type)              _AddDebugEntry(DebugInfo, #Pointer,  debug_##Type, sizeof(Type), (void*)Pointer, false)
#define DEBUG_ARRAY(Pointer, Count, Type)         _AddDebugArray(DebugInfo, #Pointer,  debug_##Type, sizeof(Type), (void*)Pointer, Count)
#define DEBUG_POINTER_ARRAY(Pointer, Count, Type) _AddDebugPointerArray(DebugInfo, #Pointer,  debug_##Type, sizeof(Type), (void*)Pointer, Count)
#define DEBUG_EDIT_VALUE(Variable, Type)          _AddDebugEntry(DebugInfo, #Variable, debug_##Type, sizeof(Type), &(Variable), true)

void UpdateAndSizeDebugEntry(game_font* Font, debug_entry* Entry, float* OutWidth, float* OutHeight) {
    float Points = DEBUG_ENTRIES_TEXT_POINTS;

    float ValueWidth = 0, ValueHeight = 0;
    if (Entry->Value) {
        switch(Entry->Type) {
            case debug_bool: {
                bool Value = *(bool*)Entry->Value;
                strcpy(Entry->ValueString, Value ? "true" : "false");
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_char: {
                char Value = *(char*)Entry->Value;
                switch (Value) {
                    case '\a': { strcpy(Entry->ValueString, "\'\\a\'"); } break;
                    case '\b': { strcpy(Entry->ValueString, "\'\\b\'"); } break;
                    case '\f': { strcpy(Entry->ValueString, "\'\\f\'"); } break;
                    case '\n': { strcpy(Entry->ValueString, "\'\\n\'"); } break;
                    case '\r': { strcpy(Entry->ValueString, "\'\\r\'"); } break;
                    case '\t': { strcpy(Entry->ValueString, "\'\\t\'"); } break;
                    case '\v': { strcpy(Entry->ValueString, "\'\\v\'"); } break;
                    case '\\': { strcpy(Entry->ValueString, "\'\\\\\'"); } break;
                    case '\'': { strcpy(Entry->ValueString, "\'\\'\'"); } break;
                    case '\"': { strcpy(Entry->ValueString, "\'\\\"\'"); } break;
                    case '\0': { strcpy(Entry->ValueString, "\'\\0\'"); } break;
                    default: {
                        if (Value >= ' ' && Value <= '~') {
                            Entry->ValueString[0] = '\'';
                            Entry->ValueString[1] = Value;
                            Entry->ValueString[2] = '\'';
                            Entry->ValueString[3] = '\0';
                        } else {
                            std::format_to(Entry->ValueString, "\'\\x{:02x}\'", (unsigned char)Value);
                        }
                    }
                }
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_string: {
                std::format_to(Entry->ValueString, "\"{}\"", (char*)Entry->Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int8: {
                int8 Value = *(int8*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int16: {
                int16 Value = *(int16*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int: {
                int Value = *(int*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int32:{
                int32 Value = *(int32*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int64:{
                int64 Value = *(int64*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_uint8:{
                uint8 Value = *(uint8*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_uint16:{
                uint16 Value = *(uint16*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_uint32:{
                uint32 Value = *(uint32*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_uint64: {
                uint64 Value = *(uint64*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_memory_index: {
                memory_index Value = *(memory_index*)Entry->Value;
                std::format_to(Entry->ValueString, "{}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_float: {
                float Value = *(float*)Entry->Value;
                std::format_to(Entry->ValueString, "{:.3f}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_double: {
                double Value = *(double*)Entry->Value;
                std::format_to(Entry->ValueString, "{:.3f}", Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_v2: {
                v2 Value = *(v2*)Entry->Value;
                std::format_to(Entry->ValueString, "V2({:.3f}, {:.3f})", Value.X, Value.Y);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_v3: {
                v3 Value = *(v3*)Entry->Value;
                std::format_to(Entry->ValueString, "V3({:.3f}, {:.3f}, {:.3f})", Value.X, Value.Y, Value.Z);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_v4: {
                v4 Value = *(v4*)Entry->Value;
                std::format_to(Entry->ValueString, "V4({:.3f}, {:.3f}, {:.3f}, {:.3f})", Value.X, Value.Y, Value.Z, Value.W);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_scale: {
                scale Value = *(scale*)Entry->Value;
                std::format_to(Entry->ValueString, "Scale({:.3f}, {:.3f}, {:.3f})", Value.X, Value.Y, Value.Z);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_quaternion: {
                quaternion Value = *(quaternion*)Entry->Value;
                std::format_to(Entry->ValueString, "{:.3f} + {:.3f}i + {:.3f}j + {:.3f}k", Value.c, Value.i, Value.j, Value.k);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_color: {
                Entry->ValueString[0] = '\0';
                ValueWidth = 2.0f * GetCharMaxHeight(Font, Points);
            } break;

            case debug_collider: {
                collider Value = *(collider*)Entry->Value;
                switch(Value.Type) {
                    case Rect_Collider: {
                        std::format_to(Entry->ValueString, "(Rect) {:.3f} x {:.3f}", Value.Rect.HalfWidth, Value.Rect.HalfHeight);
                    } break;

                    case Cube_Collider: {
                        std::format_to(Entry->ValueString, "(Cube) {:.3f} x {:.3f} x {:.3f}", Value.Cube.HalfWidth, Value.Cube.HalfHeight, Value.Cube.HalfDepth);
                    } break;

                    case Sphere_Collider: {
                        std::format_to(Entry->ValueString, "(Sphere) Radius={:.3f}", Value.Sphere.Radius);
                    } break;

                    case Capsule_Collider: {
                        std::format_to(Entry->ValueString, "(Capsule) Radius={:.3f}", Value.Capsule.Distance);
                    } break;

                    default: Raise("Invalid collider type");
                }

                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_memory_arena: {
                memory_arena Arena = *(memory_arena*)Entry->Value;
                std::format_to(Entry->ValueString, "{:.3f}", (float)Arena.Used / (float)Arena.Size);
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
                            std::format_to(Entry->ValueString, "{} ({})", EnumValue.Identifier, Value);
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
                                strcpy(Entry->ValueString, FlagValue.Identifier);
                            }
                            else {
                                strcat(Entry->ValueString, " | ");
                                strcat(Entry->ValueString, FlagValue.Identifier);
                            }

                            Matches++;
                        }
                    }
                    if (Matches == 0) {
                        strcpy(Entry->ValueString, "none");
                    }
                    std::string ValueString = std::format(" ({})", Value);
                    strcat(Entry->ValueString, ValueString.c_str());
                    GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                }
                else if (IsStructType(Entry->Type)) {
                    if (Entry->Value == 0) {
                        Entry->ValueString[0] = 'N';
                        Entry->ValueString[1] = 'U';
                        Entry->ValueString[2] = 'L';
                        Entry->ValueString[3] = 'L';
                        Entry->ValueString[4] = '\0';
                        GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                    }
                    else {
                        std::format_to(Entry->ValueString, "0x{:016x}", (uint64)Entry->Value);
                        GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                    }
                }
                else Raise("Invalid debug type.");
            }
        }
    }
    else {
        Entry->ValueString[0] = 'N';
        Entry->ValueString[1] = 'U';
        Entry->ValueString[2] = 'L';
        Entry->ValueString[3] = 'L';
        Entry->ValueString[4] = '\0';
        GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
    }

    float Width = 0,     Height = GetCharMaxHeight(Font, Points), 
      NameWidth = 0, NameHeight = 0;
    GetTextWidthAndHeight(Entry->Name, Font, Points, &Width, &Height);
    GetTextWidthAndHeight(": ", Font, Points, &NameWidth, &NameHeight);
    Width += NameWidth + ValueWidth;
    Height = fmax(Height, ValueHeight);

    debug_entry* Parent = Entry->Parent;
    while (Parent) {
        Width += 20.0f;
        Parent = Parent->Parent;
    }

    *OutWidth = Width;
    *OutHeight = Height;
}

void PushDebugEntry(render_group* Group, debug_entry* Entry, v2 Position, color Color) {
    char Buffer[128] = {};
    float Points = DEBUG_ENTRIES_TEXT_POINTS;
    float LineHeight = GetCharMaxHeight(Group->DebugFont, Points);

    v2 TextCursor = Position + V2(0, LineHeight);
    debug_entry* Parent = Entry->Parent;
    while (Parent) {
        TextCursor.X += 20.0f;
        Parent = Parent->Parent;
    }

    std::format_to(Buffer, "{}: ", Entry->Name);
    if (Entry->Type != debug_memory_arena) {
        PushText(Group, TextCursor, Buffer, .Color = Color, .Font = Group->DebugFont->ID, .Points = Points);
    }

    float Width, Height;
    GetTextWidthAndHeight(Buffer, Group->DebugFont, Points, &Width, &Height);
    TextCursor.X += Width;

    switch(Entry->Type) {
        case debug_bool: {
            bool Value = *(bool*)Entry->Value;
            PushText(Group, TextCursor, Entry->ValueString, 
                .Color = Value ? Cyan : Red, .Font = Group->DebugFont->ID, .Points = Points);
        } break;

        case debug_char:
        case debug_string:
        case debug_int8:
        case debug_int16:
        case debug_int:
        case debug_int32:
        case debug_int64:
        case debug_uint8:
        case debug_uint16:
        case debug_uint32:
        case debug_uint64:
        case debug_memory_index:
        case debug_float:
        case debug_double:
        case debug_v2:
        case debug_v3:
        case debug_v4:
        case debug_scale:
        case debug_quaternion:
        case debug_collider:
        {
            PushText(Group, TextCursor, Entry->ValueString, 
                .Color = Color, .Font = Group->DebugFont->ID, .Points = Points);
        } break;

        case debug_color: {
            rectangle Rect = { TextCursor.X, Position.Y + 3.0f, 2.0f * LineHeight, LineHeight };
            color Color = *(color*)Entry->Value;
            PushRect(Group, Rect, Color);
            PushRectOutline(Group, Rect, Gray, 1.0f);
        } break;

        case debug_memory_arena: {
            tokenizer Tokenizer = InitTokenizer(Entry->ValueString);
            float Percentage = ParseFloat(Tokenizer);
            rectangle Rect = Rectangle(Position.X, Position.Y, 450.0f, 20.0f);
            PushFillbar(Group, Entry->Name, Percentage, Rect);
        } break;

        default: {
            if (IsEnumType(Entry->Type) || IsFlagType(Entry->Type) || IsStructType(Entry->Type)) {
                PushText(Group, TextCursor, Entry->ValueString, 
                    .Color = Color, .Font = Group->DebugFont->ID, .Points = Points);
            }
        }
    }
}

#endif