#include "pch.h"
#include "debug.h"


Debug_Entry& Debug_Info::AddDebugEntry(
    String name,
    Type type,
    int size,
    void* value,
    bool editable,
    Debug_Entry* parent
) {
    Debug_Entry& entry = entries[entry_count++];
    entry.name = name;
    entry.parent = parent;
    entry.value = value;
    entry.type = type;
    entry.editable = editable;

    return entry;
}

Array<Debug_Entry> Debug_Info::AddDebugArray(
    String name,
    Type type,
    int size,
    void* value,
    uint32 count,
    Debug_Entry* parent
) {
    uint8* memory = (uint8*)value;
    Debug_Entry* result = 0;
    for (int i = 0; i < count; i++) {
        String text = String::Format(*transient, "{s}[{i}]", 2, name, i);
        if (i == 0) result = &AddDebugEntry(text, type, size, memory, false, parent);
        else                 AddDebugEntry(text, type, size, memory, false, parent);
        memory += size;
    }
    return Array<Debug_Entry>(result, count);
}

Array<Debug_Entry> Debug_Info::AddDebugPointerArray(
    String name,
    Type type,
    int size,
    void* value,
    uint32 count,
    Debug_Entry* parent
) {
    uint8* memory = (uint8*)value;
    Debug_Entry* result = 0;
    for (int i = 0; i < count; i++) {
        String text = String::Format(*transient, "{s}[{i}]", 2, name, i);
        if (i == 0) result = &AddDebugEntry(text, type, size, *(void**)memory, false, parent);
        else        AddDebugEntry(text, type, size, *(void**)memory, false, parent);
        memory += sizeof(void*);
    }
    return Array<Debug_Entry>(result, count);
}

void Debug_Info::UpdateAndSizeDebugEntry(Debug_Entry& entry, float& out_width, OUT float& out_height) {
    const float points = 20.0f;

    float value_width = 0, value_height = 0;
    if (entry.value) {
        switch(entry.type) {
            case Type_bool: {
                bool value = *(bool*)entry.value;
                entry.value_string = value ? "true" : "false";
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_char: {
                char value = *(char*)entry.value;
                switch (value) {
                    case '\a': { entry.value_string = "\'\\a\'"; } break;
                    case '\b': { entry.value_string = "\'\\b\'"; } break;
                    case '\f': { entry.value_string = "\'\\f\'"; } break;
                    case '\n': { entry.value_string = "\'\\n\'"; } break;
                    case '\r': { entry.value_string = "\'\\r\'"; } break;
                    case '\t': { entry.value_string = "\'\\t\'"; } break;
                    case '\v': { entry.value_string = "\'\\v\'"; } break;
                    case '\\': { entry.value_string = "\'\\\\\'"; } break;
                    case '\'': { entry.value_string = "\'\\'\'"; } break;
                    case '\"': { entry.value_string = "\'\\\"\'"; } break;
                    case '\0': { entry.value_string = "\'\\0\'"; } break;
                    default: {
                        if (value >= ' ' && value <= '~') {
                            char* Pointer = transient->PushArray<char>(4);
                            Pointer[0] = '\'';
                            Pointer[1] = value;
                            Pointer[2] = '\'';
                            Pointer[3] = '\0';
                            entry.value_string = Pointer;
                        } else {
                            // entry.value_string = String::Format(entry->arena, "\'\\x{:02x}\'", (unsigned char)value);
                            Log::Fatal("Invalid char.");
                        }
                    }
                }
                // // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_String: {
                entry.value_string = (char*)entry.value;
                // // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_int8: {
                int8 value = *(int8*)entry.value;
                entry.value_string = String::Format(*transient, "{i}", 1, (int32)value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_int16: {
                int16 value = *(int16*)entry.value;
                entry.value_string = String::Format(*transient, "{i}", 1, (int32)value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_int: {
                int value = *(int*)entry.value;
                entry.value_string = String::Format(*transient, "{i}", 1, value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_int32:{
                int32 value = *(int32*)entry.value;
                entry.value_string = String::Format(*transient, "{i}", 1, value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_int64:{
                int64 value = *(int64*)entry.value;
                entry.value_string = String::Format(*transient, "{I}", 1, value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_uint8:{
                uint8 value = *(uint8*)entry.value;
                entry.value_string = String::Format(*transient, "{u}", 1, (uint8)value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_uint16:{
                uint16 value = *(uint16*)entry.value;
                entry.value_string = String::Format(*transient, "{u}", 1, (uint16)value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_uint32:{
                uint32 value = *(uint32*)entry.value;
                entry.value_string = String::Format(*transient, "{u}", 1, value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_uint64: {
                uint64 value = *(uint64*)entry.value;
                entry.value_string = String::Format(*transient, "{U}", 1, value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_memory_size: {
                memory_size value = *(memory_size*)entry.value;
                entry.value_string = String::Format(*transient, "{U}", 1, value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_float: {
                float value = *(float*)entry.value;
                entry.value_string = String::Format(*transient, "{f3}", 1, value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_double: {
                double value = *(double*)entry.value;
                entry.value_string = String::Format(*transient, "{f3}", 1, value);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_V2: {
                V2 value = *(V2*)entry.value;
                entry.value_string = String::Format(*transient, "V2({f3}, {f3})", 2, value.x, value.y);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_V3: {
                V3 value = *(V3*)entry.value;
                entry.value_string = String::Format(*transient, "V3({f3}, {f3}, {f3})", 3, value.x, value.y, value.z);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_V4: {
                V4 value = *(V4*)entry.value;
                entry.value_string = String::Format(*transient, "V4({f3}, {f3}, {f3}, {f3})", 4, value.x, value.y, value.z, value.w);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_Scale: {
                Scale value = *(Scale*)entry.value;
                entry.value_string = String::Format(*transient, "Scale({f3}, {f3}, {f3})", 3, value.x, value.y, value.z);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_Quaternion: {
                Quaternion value = *(Quaternion*)entry.value;
                entry.value_string = String::Format(*transient, "{f3} + {f3}i + {f3}j + {f3}k", 4, value.c, value.i, value.j, value.k);
                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_Color: {
                entry.value_string = String();
                // value_width = 2.0f * GetCharMaxHeight(font, points);
            } break;

            case Type_Collider: {
                // Collider value = *(Collider*)entry.value;
                // switch(value.type) {
                //     case Rect_Collider: {
                //         entry.value_string = String::Format(*transient, "(Rect) {f3} x {f3}", 2, value.rect.half_width, value.rect.half_height);
                //     } break;

                //     case Cube_Collider: {
                //         entry.value_string = String::Format(*transient, "(Cube) {f3} x {f3} x {f3}", 3, value.cube.half_width, value.cube.half_height, value.cube.half_depth);
                //     } break;

                //     case Sphere_Collider: {
                //         entry.value_string = String::Format(*transient, "(Sphere) Radius={f3}", 1, value.sphere.radius);
                //     } break;

                //     case Capsule_Collider: {
                //         entry.value_string = String::Format(*transient, "(Capsule) Radius={f3}", 1, value.capsule.distance);
                //     } break;

                //     default: Raise("Invalid collider type");
                // }

                // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
            } break;

            case Type_Arena: {
                Arena arena = *(Arena*)entry.value;
                entry.value_string = String::Format(*transient, "{f3}", 1, (float)arena.used / (float)arena.capacity);
                out_width = 450.0f;
                out_height = 20.0f;
                return;
            } break;
            
            default: {
                if (IsEnumType(entry.type)) {
                    int value = *(int*)entry.value;
                    for (int i = 0; i < ENUM_VALUES_SIZE; i++) {
                        Debug_Enum_Value enum_value = ENUM_VALUES[i];
                        if (enum_value.enum_type == entry.type && enum_value.value == value) {
                            entry.value_string = String::Format(*transient, "{s} ({i})", 2, String(enum_value.identifier), value);
                            // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
                            break;
                        }
                    }
                }
                else if (IsFlagType(entry.type)) {
                    int Matches = 0;
                    int value = *(int*)entry.value;
                    char* Result = (char*)(transient->base + transient->used);
                    for (int i = 0; i < FLAG_VALUES_SIZE; i++) {
                        Debug_Enum_Value flag_value = FLAG_VALUES[i];
                        if (flag_value.enum_type == entry.type && (flag_value.value & value)) {
                            if (Matches == 0) {
                                String::Copy(*transient, flag_value.identifier);
                            }
                            else {
                                String::Copy(*transient, " | ");
                                String::Copy(*transient, flag_value.identifier);
                            }

                            Matches++;
                        }
                    }
                    if (Matches == 0) {
                        String::Copy(*transient, "none");
                    }
                    String::Format(*transient, " ({i})", 1, value);
                    entry.value_string = Result;
                    // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
                }
                else if (IsStructType(entry.type)) {
                    if (entry.value == 0) {
                        entry.value_string = "NULL";
                        // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
                    }
                    else {
                        entry.value_string = String::Format(*transient, "0x{x}", (uint64)entry.value);
                        // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
                    }
                }
                else Log::Fatal("Invalid debug type.");
            }
        }
    }
    else {
        entry.value_string = "NULL";
        // GetTextWidthAndHeight(entry.value_string, font, points, &value_width, &value_height);
    }

    // float width = 0,      height = GetCharMaxHeight(font, points), 
    // name_width = 0, name_height = 0;
    // GetTextWidthAndHeight(entry->name, font, points, &width, &height);
    // GetTextWidthAndHeight(": ", font, points, &name_width, &name_height);
    // width += name_width + value_width;
    // height = fmax(height, value_height);

    // Debug_Entry* parent = entry->parent;
    // while (parent) {
    //     width += 20.0f;
    //     parent = parent->parent;
    // }

    // *out_width = width;
    // *out_height = height;
}

// void PushDebugEntry(Arena& transient, Render_Group* group, Debug_Entry* entry, V2 position, Color color) {
//     float points = DEBUG_ENTRIES_TEXT_POINTS;
//     // float line_height = GetCharMaxHeight(group->debug_font, points);

//     V2 text_cursor = position + V2(0, line_height);
//     Debug_Entry* parent = entry->parent;
//     while (parent) {
//         text_cursor.x += 20.0f;
//         parent = parent->parent;
//     }

//     String name = String::Format(transient, "{s}: ", 1, entry->name);
//     if (entry->type != Type_Arena) {
//         // PushText(group, text_cursor, name, .color = color, .font = group->debug_font, .points = points);
//     }

//     float width, height;
//     // GetTextWidthAndHeight(name, group->debug_font, points, &width, &height);
//     text_cursor.x += width;

//     switch(entry->type) {
//         case Type_bool: {
//             bool value = *(bool*)entry.value;
//             // PushText(group, text_cursor, entry.value_string, 
//             //     .color = value ? color::CYAN : color::RED, .Font = Group->DebugFont, .points = points);
//         } break;

//         case Type_char:
//         case Type_String:
//         case Type_int8:
//         case Type_int16:
//         case Type_int:
//         case Type_int32:
//         case Type_int64:
//         case Type_uint8:
//         case Type_uint16:
//         case Type_uint32:
//         case Type_uint64:
//         case Type_memory_size:
//         case Type_float:
//         case Type_double:
//         case Type_V2:
//         case Type_V3:
//         case Type_V4:
//         case Type_Scale:
//         case Type_Quaternion:
//         case Type_Collider:
//         {
//             // PushText(group, text_cursor, entry.value_string, 
//             //     .color = color, .font = group->debug_font, .points = points);
//         } break;

//         case Type_Color: {
//             Color color = *(Color*)entry.value;
//             PushRect(group, text_cursor.x, position.y + 3.0f, 2.0f * line_height, line_height, color);
//             // PushRectOutline(group, rect, Color::GRAY, 1.0f);
//         } break;

//         case Type_Arena: {
//             Tokenizer tokenizer = InitTokenizer(entry.value_string.content);
//             float percentage = ParseFloat(tokenizer);
//             Rect rect = { position.x, position.y, 450.0f, 20.0f };
//             // PushFillbar(group, entry->name.content, Percentage, Rect);
//         } break;

//         default: {
//             if (IsEnumType(entry->type) || IsFlagType(entry->type) || IsStructType(entry->type)) {
//                 // PushText(group, text_cursor, entry.value_string, 
//                 //     .color = color, .font = group->debug_font, .points = points);
//             }
//         }
//     }
// }
