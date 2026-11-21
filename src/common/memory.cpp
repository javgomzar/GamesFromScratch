#include "memory_context.h"

Arena Memory::permanent = Arena(permanent_size, Platform::AllocateMemory(permanent_size));
Arena Memory::transient = Arena(transient_size, Platform::AllocateMemory(transient_size));
