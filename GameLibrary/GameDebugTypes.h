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
    Debug_Type_float,
    Debug_Type_double,
    Debug_Type_v2,
    Debug_Type_v3,
    Debug_Type_v4,
    Debug_Type_scale,
    Debug_Type_quaternion,
    Debug_Type_color,
    Debug_Type_collider,
};

bool IsEnumType(debug_type Type) { return Type > 20 && Type < 21; }
bool IsStructType(debug_type Type) { return Type > 20 && Type < 21; }

struct debug_enum_value {
    debug_type EnumType;
    char* Identifier;
    int Value;
};

const int ENUM_VALUES_SIZE = 0;
debug_enum_value* EnumValues = 0;

struct debug_struct_member {
    char* Name;
    debug_type StructType;
    debug_type MemberType;
    uint64 Size;
    uint64 Offset;
    int ArraySize;
    bool IsPointer;
};

const int STRUCT_MEMBERS_SIZE = 0;
debug_struct_member* StructMembers = 0;

