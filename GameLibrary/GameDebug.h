#ifndef GAME_DEBUG
#define GAME_DEBUG

#define Introspect(params)

inline void Assert(bool assertion, const char* Message = "") {
    if (!assertion) {
        int* i = 0;
        int j = *i;
    }
}

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Debug values                                                                                                                    |
// +---------------------------------------------------------------------------------------------------------------------------------+

enum debug_type {
    Debug_Type_bool,
    Debug_Type_int,
    Debug_Type_int32,
    Debug_Type_int64,
    Debug_Type_uint32,
    Debug_Type_uint64,
    Debug_Type_float,
    Debug_Type_double,
    Debug_Type_color,
};

struct debug_entry {
    char* Name;
    debug_type Type;
    void* Value;
    int Size;
    bool Editable;
    char ValueString[16];
    float Left, Top, Width, Height;
};

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Debug info                                                                                                                      |
// +---------------------------------------------------------------------------------------------------------------------------------+

const int MAX_DEBUG_ENTRIES = 32;
struct debug_info {
    debug_entry Entries[MAX_DEBUG_ENTRIES];
    int nEntries;
};

void ClearDebugContext(debug_info* DebugInfo) {
    DebugInfo->nEntries = 0;
}

void _AddDebugEntry(debug_info* DebugInfo, char* Name, debug_type Type, int Size, void* Value, bool Editable) {
    DebugInfo->Entries[DebugInfo->nEntries++] = {
        Name, Type, Value, Size, Editable
    };
}

#define DEBUG_VALUE(Variable, Type)      _AddDebugEntry(DebugInfo, #Variable, Debug_Type_##Type, sizeof(Type), &##Variable, false)
#define DEBUG_EDIT_VALUE(Variable, Type) _AddDebugEntry(DebugInfo, #Variable, Debug_Type_##Type, sizeof(Type), &##Variable, true)

#endif