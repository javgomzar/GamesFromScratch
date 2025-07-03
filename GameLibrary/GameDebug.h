#ifndef GAME_DEBUG
#define GAME_DEBUG

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Debug values                                                                                                                    |
// +---------------------------------------------------------------------------------------------------------------------------------+

#include "GameDebugTypes.h"

struct debug_entry {
    char* Name;
    debug_type Type;
    void* Value;
    bool Editable;
    debug_entry* Parent;
    char ValueString[64];
    float Width, Height;
};

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Debug info                                                                                                                      |
// +---------------------------------------------------------------------------------------------------------------------------------+

const int MAX_DEBUG_ENTRIES = 128;
struct debug_info {
    debug_entry Entries[MAX_DEBUG_ENTRIES];
    int nEntries;
};

void ClearDebugContext(debug_info* DebugInfo) {
    DebugInfo->nEntries = 0;
}

void _AddDebugEntry(debug_info* DebugInfo, char* Name, debug_type Type, int Size, void* Value, bool Editable, debug_entry* Parent = 0) {
    debug_entry* Entry = &DebugInfo->Entries[DebugInfo->nEntries++];
    *Entry = {
        Name, Type, Value, Editable, Parent
    };
    if (IsStructType(Type) && Value != NULL) {
        bool Found = false;
        for (int i = 0; i < STRUCT_MEMBERS_SIZE; i++) {
            debug_struct_member Member = StructMembers[i];
            if (Member.StructType == Type) {
                if (!Found) Found = true;
                while (Member.StructType == Type) {
                    uint8* Pointer = (uint8*)Value + Member.Offset;
                    if (Member.IsPointer) {
                        if (Pointer) {
                            _AddDebugEntry(DebugInfo, Member.Name, Member.MemberType, Member.Size, *(void**)Pointer, Editable, Entry);
                        }
                        else {
                            _AddDebugEntry(DebugInfo, Member.Name, Member.MemberType, Member.Size, 0, Editable, Entry);
                        }
                    }
                    else if (Member.ArraySize > 0) {
                        for (int j = 0; j < Member.ArraySize; j++) {
                            _AddDebugEntry(DebugInfo, Member.Name, Member.MemberType, Member.Size, (void*)Pointer, Editable, Entry);
                            Pointer += Member.Size;
                        }
                    }
                    else {
                        _AddDebugEntry(DebugInfo, Member.Name, Member.MemberType, Member.Size, (void*)Pointer, Editable, Entry);
                    }
                    i++;
                    if (i < STRUCT_MEMBERS_SIZE) Member = StructMembers[i];
                    else break;
                }
                break;
            }
        }
    }
}

#define DEBUG_POINTER(Pointer, Type)     _AddDebugEntry(DebugInfo, #Pointer, Debug_Type_##Type, sizeof(Type), (void*)##Pointer,  false)
#define DEBUG_VALUE(Variable, Type)      _AddDebugEntry(DebugInfo, #Variable, Debug_Type_##Type, sizeof(Type), &##Variable, false)
#define DEBUG_EDIT_VALUE(Variable, Type) _AddDebugEntry(DebugInfo, #Variable, Debug_Type_##Type, sizeof(Type), &##Variable, true)

#endif