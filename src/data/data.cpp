#include "pch.h"
#include "data.h"


void Data_Manager::Initialize(String path) {
    uint8* memory = (uint8*)Platform::AllocateMemory(TOTAL_DATA_MEMORY);
    for (Frame_ID id = 0; id < FRAME_COUNT; ++id) {
        frames[id].frame_id = id;
        frames[id].page_id = 0;
        frames[id].pins = 0;
        frames[id].data = memory + id * PAGE_SIZE;
        frames[id].dirty = false;
    }

    free_frames = Stack<Frame_ID>(FRAME_COUNT);
    for (Frame_ID id = FRAME_COUNT - 1; id >= 0; --id) {
        free_frames.Push(id);
    }

    database_file_path = path;
    if (Platform::FileExists(path)) {
        
    }
}