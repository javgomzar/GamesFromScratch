#pragma once

#include "struct.h"
#include <render.h>
#include <data_structures.h>


struct Debug_Entry {
    String name;
    Debug_Entry* parent = nullptr;
    void* value = nullptr;
    String value_string;
    Type type = Type_void;
    bool editable = false;
};

inline const int MAX_DEBUG_ENTRIES = 128;
struct Debug_Info {
    static Debug_Entry entries[MAX_DEBUG_ENTRIES];
    static Arena* transient;
    static Font* font;
    static int entry_count;
    static Debug_Entry& AddDebugEntry(
        String name, 
        Type type, 
        int size, 
        void* value, 
        bool editable, 
        Debug_Entry* parent = nullptr
    );
    static Array<Debug_Entry> AddDebugArray(
        String name, 
        Type type, 
        int size, 
        void* value, 
        uint32 count, 
        Debug_Entry* parent = nullptr
    );
    static Array<Debug_Entry> AddDebugPointerArray(
        String name, 
        Type type, 
        int size, 
        void* value, 
        uint32 count, 
        Debug_Entry* parent = nullptr
    );
    static void UpdateAndSizeDebugEntry(Debug_Entry& entry, float& out_width, OUT float& out_height);
};

#define DEBUG_VALUE(variable, type)               Debug_Info::AddDebugEntry(#variable, Type_##type, sizeof(type), &(variable), false)
#define DEBUG_POINTER(pointer, type)              Debug_Info::AddDebugEntry(#pointer,  Type_##type, sizeof(type), (void*)pointer, false)
#define DEBUG_ARRAY(pointer, count, type)         Debug_Info::AddDebugArray(#pointer,  Type_##type, sizeof(type), (void*)pointer, count)
#define DEBUG_POINTER_ARRAY(pointer, count, type) Debug_Info::AddDebugPointerArray(#pointer, Type_##type, sizeof(type), (void*)pointer, count)
#define DEBUG_EDIT_VALUE(variable, type)          Debug_Info::AddDebugEntry(#variable, Type_##type, sizeof(type), &(variable), true)
