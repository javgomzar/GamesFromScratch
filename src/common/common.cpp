#include "common.h"


void ZeroSize(memory_size size, void* memory) {
    if (size > 0) {
        memset(memory, 0, size);
    }
}

uint16 BigEndian(uint16 little_endian) {
    return (little_endian << 8) | (little_endian >> 8);
}

uint32 BigEndian(uint32 little_endian) {
    return ((little_endian >> 24) & 0x000000FF) |
           ((little_endian >> 8)  & 0x0000FF00) |
           ((little_endian << 8)  & 0x00FF0000) |
           ((little_endian << 24) & 0xFF000000);
}

uint64 BigEndian(uint64 little_endian) {
    return ((little_endian >> 56) & 0x00000000000000FF) |
           ((little_endian >> 40) & 0x000000000000FF00) |
           ((little_endian >> 24) & 0x0000000000FF0000) |
           ((little_endian >> 8)  & 0x00000000FF000000) |
           ((little_endian << 8)  & 0x000000FF00000000) |
           ((little_endian << 24) & 0x0000FF0000000000) |
           ((little_endian << 40) & 0x00FF000000000000) |
           ((little_endian << 56) & 0xFF00000000000000);
}

int16 BigEndian(int16 little_endian) {
    uint16 Unsigned = BigEndian(*(uint16*)&little_endian);
    return *(int16*)&Unsigned;
}

int32 BigEndian(int32 little_endian) {
    uint32 Unsigned = BigEndian(*(uint32*)&little_endian);
    return *(int32*)&Unsigned;
}

int64 BigEndian(int64 little_endian) {
    uint64 Unsigned = BigEndian(*(uint64*)&little_endian);
    return *(int64*)&Unsigned;
}

inline uint8 MSB32(uint32 x) {
	unsigned long result = 0;
#ifdef _WIN32
	_BitScanReverse(&result, x);
#else
    result = 31 - __builtin_clz(x);
#endif
	return (uint32)result;
}

inline uint8 MSB64(uint64 x) {
	unsigned long result = 0;
#ifdef _WIN32
	_BitScanReverse64(&result, x);
#else
    result = 31 - __builtin_clzll(x);
#endif
	return (uint32)result;
}

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Memory arenas                                                                                                                   |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

Arena::Arena(memory_size size) : capacity(size), base((uint8*)calloc(1, size)), used(0) {}

Arena::Arena(memory_size size, void* memory) : capacity(size), base((uint8*)memory), used(0) {}

Arena::Arena(Arena& arena, memory_size size) : capacity(size), base((uint8*)arena.PushSize(size)), used(0) {}

Arena::~Arena() {
    capacity = 0;
    used = 0;
    base = nullptr;
}

void Arena::Clear() {
    ZeroSize(used, base);
    used = 0;
}

void Arena::Free() {
    free(base);
    base = nullptr;
    capacity = 0;
    used = 0;
}

void* Arena::PushSize(memory_size size) {
    assert(capacity >= used + size);
    void* result = base + used;
    used += size;
    return result;
}

void Arena::PopSize(memory_size size) {
    memory_size bytes_erased = size < used ? size : used;
    void* result = (void*)(base + used - bytes_erased);
    ZeroSize(bytes_erased, result);
    used -= bytes_erased;
}

/*
+---------------------------------------------------------------------------------------------------------------------------------+
| Strings                                                                                                                         |
+---------------------------------------------------------------------------------------------------------------------------------+
*/

String::String() : length(0), content(nullptr) {}

String::String(const char* str) : length(strlen(str)), content(str) {}

String::String(const char* str, int64 len) : length(len), content(str) {}

char String::operator[](int64 index) const {
    if (index < -length || index > length) {
        return '\0';
    }
    if (index < 0) {
        return content[length + index];
    }
    return content[index];
};

bool String::operator==(String rhs) const {
    bool result = length == rhs.length;
    if (result) {
        for (int64 i = 0; i < length; ++i) {
            if ((*this)[i] != rhs[i]) {
                return false;
            }
        }
    }
    return result;
}

String String::Slice(int start, int end) const {
    assert(start <= end);
    return String(content + start, end - start);
}

char* String::Copy(Arena& arena, String string) {
    char* result = arena.PushArray<char>(string.length);
    strncpy(result, string.content, string.length);
    return result;
}

String String::Persist(Arena& arena, String string) {
    int* length = arena.Push<int>();
    *length = string.length;
    char* content = Copy(arena, string);
    return String(content, string.length);
}

String String::Read(void* memory) {
    int* length = (int*)memory;
    return String((char*)(length + 1), *length);
}

/*
    Split a `String` given a separator. Beware that the `input` parameter will be modified.
*/
String String::Split(String& input, char separator) {
    String result;
    result.length = 0;
    result.content = input.content;
    for (int i = 0; i < input.length; i++) {
        if (input[i] == separator) {
            return result;
        }
        else {
            input.content++;
            input.length--;
            result.length++;
        }
    }
    return result;
}

String String::Format(Arena& arena, const char* format, int input_count, ...) {
    String result = {};
    int format_length = strlen(format);

    if (format_length == 0) {
        return result;
    }

    char* pointer = (char*)(arena.base + arena.used);
    result.content = pointer;

    va_list args;
    va_start(args, input_count);

    int dest_index = 0;
    for (int source_index = 0; source_index < format_length; source_index++) {
        if (format[source_index] == '{') {
            switch (format[++source_index]) {
                case 'i': {
                    int32 input = va_arg(args, int32);
                    if (input == 0) {
                        arena.Push<char>();
                        pointer[dest_index++] = '0';
                        result.length++;
                        break;
                    }
                    else if (input < 0) {
                        arena.Push<char>();
                        pointer[dest_index++] = '-';
                        result.length++;
                        input = -input;
                    }
                    int digit_count = 0;
                    char digits[16];
                    while (input > 0) {
                        digits[digit_count++] = '0' + (input % 10);
                        input /= 10;
                    }
                    for (int i = 0; i < digit_count; i++) {
                        arena.Push<char>();
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                        result.length++;
                    }
                } break;

                case 'I': {
                    int64 input = va_arg(args, int64);
                    if (input == 0) {
                        arena.Push<char>();
                        pointer[dest_index++] = '0';
                        result.length++;
                        break;
                    }
                    else if (input < 0) {
                        arena.Push<char>();
                        pointer[dest_index++] = '-';
                        result.length++;
                        input = -input;
                    }
                    int digit_count = 0;
                    char digits[32];
                    while (input > 0) {
                        digits[digit_count++] = '0' + (input % 10);
                        input /= 10;
                    }
                    for (int i = 0; i < digit_count; i++) {
                        arena.Push<char>();
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                        result.length++;
                    }
                } break;

                case 'u': {
                    uint32 input = va_arg(args, uint32);
                    if (input == 0) {
                        arena.Push<char>();
                        pointer[dest_index++] = '0';
                        result.length++;
                        break;
                    }
                    int digit_count = 0;
                    char digits[16];
                    while (input > 0) {
                        digits[digit_count++] = '0' + (input % 10);
                        input /= 10;
                    }
                    for (int i = 0; i < digit_count; i++) {
                        arena.Push<char>();
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                        result.length++;
                    }
                } break;

                case 'U': {
                    uint64 input = va_arg(args, uint64);
                    if (input == 0) {
                        arena.Push<char>();
                        pointer[dest_index++] = '0';
                        result.length++;
                        break;
                    }
                    int digit_count = 0;
                    char digits[32];
                    while (input > 0) {
                        digits[digit_count++] = '0' + (input % 10);
                        input /= 10;
                    }
                    for (int i = 0; i < digit_count; i++) {
                        arena.Push<char>();
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                        result.length++;
                    }
                } break;

                case 'x': {
                    uint64 input = va_arg(args, uint64);
                    arena.PushArray<char>(18);
                    pointer[dest_index++] = '0';
                    pointer[dest_index++] = 'x';
                    for (int i = 15; i >= 0; i--) {
                        uint64 r = input % 16;
                        if (r < 10) {
                            pointer[dest_index + i] = '0' + r;
                        }
                        else {
                            pointer[dest_index + i] = 'a' + (r - 10);
                        }
                        input >>= 4;
                    }
                    dest_index += 16;
                    result.length += 18;
                } break;

                case 'f': {
                    double input = va_arg(args, double);
                    if (input < 0) {
                        input = -input;
                        arena.Push<char>();
                        pointer[dest_index++] = '-';
                        result.length++;
                    }

                    uint64 integer_part = floor(input);
                    double decimal_part = input - integer_part;

                    if (integer_part == 0) {
                        arena.Push<char>();
                        pointer[dest_index++] = '0';
                        result.length++;
                    }

                    int digit_count = 0;
                    char digits[32];
                    while (integer_part > 0) {
                        digits[digit_count++] = '0' + (integer_part % 10);
                        integer_part /= 10;
                    }
                    
                    arena.PushArray<char>(digit_count + 1);
                    for (int i = 0; i < digit_count; i++) {
                        pointer[dest_index++] = digits[digit_count - (i + 1)];
                    }
                    pointer[dest_index++] = '.';
                    result.length += digit_count + 1;

                    int decimal_count = format[++source_index] - '0';
                    for (int i = 0; i < decimal_count; i++) {
                        decimal_part *= 10.0;
                        int digit = decimal_part;
                        arena.Push<char>();
                        pointer[dest_index++] = '0' + digit;
                        result.length++;
                        decimal_part -= digit;
                    }
                } break;

                case 's': {
                    String input = va_arg(args, String);
                    result.length += input.length;
                    arena.PushArray<char>(input.length);
                    for (int i = 0; i < input.length; i++) {
                        pointer[dest_index++] = input[i];
                    }
                } break;

                default: {
                    assert(false);
                }
            }

            assert(format[source_index + 1] == '}');
            source_index++;
        }
        else {
            arena.Push<char>();
            pointer[dest_index++] = format[source_index];
            result.length++;
        }
    }
    arena.Push<char>();
    va_end(args);
    return result;
}
