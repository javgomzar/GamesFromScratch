#include "common.h"
#include <platform.h>

struct Memory {
    static Arena permanent;
    static Arena transient;
    static memory_size permanent_size;
    static memory_size transient_size;
};
