#include "transient.h"


Arena Transient::arena = Arena(size, Platform::AllocateMemory(size));
