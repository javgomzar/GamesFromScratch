#include "pch.h"
#include "debug.h"


Debug_Entry& Debug_Info::AddDebugEntry(
    String name,
    Type type,
    int size,
    void* value,
    bool editable,
    Debug_Entry* parent = nullptr
) {
    Debug_Entry& entry = entries[entry_count++];
    entry.name = name;
    entry.parent = parent;
    entry.value = value;
    entry.type = type;
    entry.editable = editable;

    return entry;
}