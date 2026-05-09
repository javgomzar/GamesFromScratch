#ifndef GAME_DEBUG
#define GAME_DEBUG

#include "GameStructs.h"
#include "GameRender.h"

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Debug info                                                                                                                      |
// +---------------------------------------------------------------------------------------------------------------------------------+

struct debug_entry {
    string Name;
    debug_entry* Parent;
    memory_arena* Arena;
    void* Value;
    string ValueString;
    debug_type Type;
    bool Editable;
};

const int MAX_DEBUG_ENTRIES = 128;
struct debug_info {
    debug_entry Entries[MAX_DEBUG_ENTRIES];
    memory_arena* Arena;
    int nEntries;
    float FPS;
    float UsedTime;
    float UsedMCyclesPerFrame;
    float UsedMemory;
};

debug_entry* _AddDebugEntry(
    debug_info* DebugInfo, 
    string Name, 
    debug_type Type, 
    int Size, 
    void* Value, 
    bool Editable, 
    debug_entry* Parent = nullptr
) {
    debug_entry* Entry = &DebugInfo->Entries[DebugInfo->nEntries++];
    Entry->Name = Name;
    Entry->Parent = Parent;
    Entry->Arena = DebugInfo->Arena;
    Entry->Value = Value;
    Entry->Type = Type;
    Entry->Editable = Editable;

    return Entry;
}

debug_entry* _AddDebugArray(
    debug_info* DebugInfo,
    string Name,
    debug_type Type,
    int Size,
    void* Value,
    uint32 Count,
    debug_entry* Parent = nullptr
) {
    uint8* Memory = (uint8*)Value;
    debug_entry* Result = 0;
    for (int i = 0; i < Count; i++) {
        string Text = Format(DebugInfo->Arena, "{s}[{i}]", 2, Name, i);
        if (i == 0) Result = _AddDebugEntry(DebugInfo, Text, Type, Size, Memory, false, Parent);
        else        _AddDebugEntry(DebugInfo, Text, Type, Size, Memory, false, Parent);
        Memory += Size;
    }
    return Result;
}

debug_entry* _AddDebugPointerArray(
    debug_info* DebugInfo,
    string Name,
    debug_type Type,
    int Size,
    void* Value,
    uint32 Count,
    debug_entry* Parent = nullptr
) {
    uint8* Memory = (uint8*)Value;
    debug_entry* Result = 0;
    for (int i = 0; i < Count; i++) {
        string Text = Format(DebugInfo->Arena, "{s}[{i}]", 2, Name, i);
        if (i == 0) Result = _AddDebugEntry(DebugInfo, Text, Type, Size, *(void**)Memory, false, Parent);
        else        _AddDebugEntry(DebugInfo, Text, Type, Size, *(void**)Memory, false, Parent);
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
                Entry->ValueString = Value ? "true" : "false";
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_char: {
                char Value = *(char*)Entry->Value;
                switch (Value) {
                    case '\a': { Entry->ValueString = "\'\\a\'"; } break;
                    case '\b': { Entry->ValueString = "\'\\b\'"; } break;
                    case '\f': { Entry->ValueString = "\'\\f\'"; } break;
                    case '\n': { Entry->ValueString = "\'\\n\'"; } break;
                    case '\r': { Entry->ValueString = "\'\\r\'"; } break;
                    case '\t': { Entry->ValueString = "\'\\t\'"; } break;
                    case '\v': { Entry->ValueString = "\'\\v\'"; } break;
                    case '\\': { Entry->ValueString = "\'\\\\\'"; } break;
                    case '\'': { Entry->ValueString = "\'\\'\'"; } break;
                    case '\"': { Entry->ValueString = "\'\\\"\'"; } break;
                    case '\0': { Entry->ValueString = "\'\\0\'"; } break;
                    default: {
                        if (Value >= ' ' && Value <= '~') {
                            char* Pointer = PushArray(Entry->Arena, 4, char);
                            Pointer[0] = '\'';
                            Pointer[1] = Value;
                            Pointer[2] = '\'';
                            Pointer[3] = '\0';
                            Entry->ValueString = Pointer;
                        } else {
                            // Entry->ValueString = Format(Entry->Arena, "\'\\x{:02x}\'", (unsigned char)Value);
                            Raise("Invalid char.");
                        }
                    }
                }
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_string: {
                Entry->ValueString = (char*)Entry->Value;
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int8: {
                int8 Value = *(int8*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{i}", 1, (int32)Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int16: {
                int16 Value = *(int16*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{i}", 1, (int32)Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int: {
                int Value = *(int*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{i}", 1, Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int32:{
                int32 Value = *(int32*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{i}", 1, Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_int64:{
                int64 Value = *(int64*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{I}", 1, Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_uint8:{
                uint8 Value = *(uint8*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{u}", 1, (uint8)Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_uint16:{
                uint16 Value = *(uint16*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{u}", 1, (uint16)Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_uint32:{
                uint32 Value = *(uint32*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{u}", 1, Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_uint64: {
                uint64 Value = *(uint64*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{U}", 1, Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_memory_index: {
                memory_index Value = *(memory_index*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{U}", 1, Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_float: {
                float Value = *(float*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{f3}", 1, Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_double: {
                double Value = *(double*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{f3}", 1, Value);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_v2: {
                v2 Value = *(v2*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "V2({f3}, {f3})", 2, Value.X, Value.Y);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_v3: {
                v3 Value = *(v3*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "V3({f3}, {f3}, {f3})", 3, Value.X, Value.Y, Value.Z);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_v4: {
                v4 Value = *(v4*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "V4({f3}, {f3}, {f3}, {f3})", 4, Value.X, Value.Y, Value.Z, Value.W);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_scale: {
                scale Value = *(scale*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "Scale({f3}, {f3}, {f3})", 3, Value.X, Value.Y, Value.Z);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_quaternion: {
                quaternion Value = *(quaternion*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{f3} + {f3}i + {f3}j + {f3}k", 4, Value.c, Value.i, Value.j, Value.k);
                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_color: {
                Entry->ValueString = string();
                ValueWidth = 2.0f * GetCharMaxHeight(Font, Points);
            } break;

            case debug_collider: {
                collider Value = *(collider*)Entry->Value;
                switch(Value.Type) {
                    case Rect_Collider: {
                        Entry->ValueString = Format(Entry->Arena, "(Rect) {f3} x {f3}", 2, Value.Rect.HalfWidth, Value.Rect.HalfHeight);
                    } break;

                    case Cube_Collider: {
                        Entry->ValueString = Format(Entry->Arena, "(Cube) {f3} x {f3} x {f3}", 3, Value.Cube.HalfWidth, Value.Cube.HalfHeight, Value.Cube.HalfDepth);
                    } break;

                    case Sphere_Collider: {
                        Entry->ValueString = Format(Entry->Arena, "(Sphere) Radius={f3}", 1, Value.Sphere.Radius);
                    } break;

                    case Capsule_Collider: {
                        Entry->ValueString = Format(Entry->Arena, "(Capsule) Radius={f3}", 1, Value.Capsule.Distance);
                    } break;

                    default: Raise("Invalid collider type");
                }

                GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
            } break;

            case debug_memory_arena: {
                memory_arena Arena = *(memory_arena*)Entry->Value;
                Entry->ValueString = Format(Entry->Arena, "{f3}", 1, (float)Arena.Used / (float)Arena.Size);
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
                            Entry->ValueString = Format(Entry->Arena, "{s} ({i})", 2, string(EnumValue.Identifier), Value);
                            GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                            break;
                        }
                    }
                }
                else if (IsFlagType(Entry->Type)) {
                    int Matches = 0;
                    int Value = *(int*)Entry->Value;
                    char* Result = (char*)(Entry->Arena->Base + Entry->Arena->Used);
                    for (int i = 0; i < FLAG_VALUES_SIZE; i++) {
                        debug_enum_value FlagValue = FlagValues[i];
                        if (FlagValue.EnumType == Entry->Type && (FlagValue.Value & Value)) {
                            if (Matches == 0) {
                                PushString(Entry->Arena, FlagValue.Identifier);
                            }
                            else {
                                PushString(Entry->Arena, " | ");
                                PushString(Entry->Arena, FlagValue.Identifier);
                            }

                            Matches++;
                        }
                    }
                    if (Matches == 0) {
                        PushString(Entry->Arena, "none");
                    }
                    Format(Entry->Arena, " ({i})", 1, Value);
                    Entry->ValueString = Result;
                    GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                }
                else if (IsStructType(Entry->Type)) {
                    if (Entry->Value == 0) {
                        Entry->ValueString = "NULL";
                        GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                    }
                    else {
                        Entry->ValueString = Format(Entry->Arena, "0x{x}", (uint64)Entry->Value);
                        GetTextWidthAndHeight(Entry->ValueString, Font, Points, &ValueWidth, &ValueHeight);
                    }
                }
                else Raise("Invalid debug type.");
            }
        }
    }
    else {
        Entry->ValueString = "NULL";
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
    float Points = DEBUG_ENTRIES_TEXT_POINTS;
    float LineHeight = GetCharMaxHeight(Group->DebugFont, Points);

    v2 TextCursor = Position + V2(0, LineHeight);
    debug_entry* Parent = Entry->Parent;
    while (Parent) {
        TextCursor.X += 20.0f;
        Parent = Parent->Parent;
    }

    string Name = Format(Group->Transient, "{s}: ", 1, Entry->Name);
    if (Entry->Type != debug_memory_arena) {
        PushText(Group, TextCursor, Name, .Color = Color, .Font = Group->DebugFont->ID, .Points = Points);
    }

    float Width, Height;
    GetTextWidthAndHeight(Name, Group->DebugFont, Points, &Width, &Height);
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
            tokenizer Tokenizer = InitTokenizer(Entry->ValueString.Content);
            float Percentage = ParseFloat(Tokenizer);
            rectangle Rect = Rectangle(Position.X, Position.Y, 450.0f, 20.0f);
            PushFillbar(Group, Entry->Name.Content, Percentage, Rect);
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