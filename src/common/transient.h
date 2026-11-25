#include "common.h"
#include <platform.h>


struct Transient {
    static Arena arena;
    static memory_size size;
};
