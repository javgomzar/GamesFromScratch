#pragma once

#include "common.h"


/*
    Memory for elements must be previously allocated.
*/
template <typename T> class Stack {
private:
    T* top;
    T* base;
public:
    uint64 n;
    uint64 capacity;

    Stack(uint64 max_size = 32) {
        n = 0;
        T* memory = (T*)calloc(32, sizeof(T));
        top = memory;
        base = memory;
        capacity = max_size;
    }

    ~Stack() {
        free(top);
        n = 0;
        capacity = 0;
    }

    void Push(T element) {
        if (n < capacity) {
            *top++ = element;
            n++;
        }
        else Assert(false, "Stack is full.");
    }

    T Pop() {
        if (n > 0) {
            top--;
            T result = *top;
            *top = {};
            n--;
            return result;
        }
        return {};
    }

    void Clear() {
        ZeroSize(n * sizeof(T), top - n);
        top -= n;
        n = 0;
    }

    const T& operator[](uint32 i) const {
        Assert(0 <= i && i < n);
        return base[i];
    }
};

template <typename T> class Link {
private:
    Link<T>* previous;
    Link<T>* next;
public:
    T* data;

    void Attach(Link& rhs) {
        next = &rhs;
        rhs.previous = this;
    }

    void Delete() {
        Attach(*previous, next);
        previous = nullptr;
        next = nullptr;
    }
};

/*
    Doubly-linked list. All links must have been allocated somewhere previously.
*/
template <typename T> class Linked_List {
public:
    Link<T>* first;
    Link<T>* last;

    void PushBack(Link<T>* element) {
        if (!first || !last) {
            first = element;
        }
        else {
            Attach(last, element);
        }
        last = element;
    }

    void PushFront(struct Link<T>* element) {
        if (!first || !last) {
            last = element;
        }
        else {
            Attach(element, first);
        }
        first = element;
    }

    void MakeCircular() {
        Attach(last, first);
    }

    void Break(Link<T>* link) {
        if (first == link) first = link->next;
        if (last == link)  last  = link->previous;
        Delete(link);
    }

    bool IsEmpty() {
        return !first && !last;
    }

    uint64 Length(Linked_List list) {
        Link* link = list.first;
        uint64 result = 0;
        do {
            link = link->next;
            result++;
        } while (link && link != list.first);
        return result;
    }

    void Concatenate(Linked_List rhs) {
        Attach(last, rhs.first);
        last = rhs.last;
        return result;
    }
};


/*
    Fixed length array.
*/
template <typename T> class Array {
private:
    T* data;
public:
    uint32 length;

    Array() {
        length = 0;
        data = nullptr;
    }

    Array(Arena& arena, uint32 N) {
        length = N;
        data = PushArray(arena, N, T);
    }

    ~Array() {
        length = 0;
        data = nullptr;
    }

    T& operator[](int64 index) const {
        if (-index > length || index >= length) {
            Log(Log_Level::Error, "Array index out of bounds.");
        }
        else if (index >= 0) {
            return data[index];
        }
        else if (index < 0) {
            return data[length - index];
        }
    }
};

/*
    Fixed length lists that track available slots.
*/
template <typename T> class Free_List {
private:
    T* slots;
    uint32* free_slots;
public:
    bool* occupied;
    uint32 size = 0;
    uint32 count = 0;
    uint32 free_count = 0;

    Free_List(Arena& arena, uint32 n) {
        size = n;
        slots = PushArray(arena, n, T);
        free_slots = PushArray(arena, n, uint32);
        occupied = PushArray(arena, n, bool);
    }

    uint32 Insert(T element) {
        Assert(count < size);
        
        uint32 result;
        if (free_count > 0) {
            result = free_slots[free_count--];
        }
        else {
            result = count++;
        }

        slots[result] = element;
        occupied[result] = true;
        return result;
    }

    T& operator[](uint32 index) {
        return slots[index];
    }

    T operator[](uint32 index) const {
        return slots[index];
    }

    void Remove(uint32 index) {
        if (occupied[index]) {
            free_slots[free_count++] = index;
            occupied[index] = false;
            count--;
        }
    }
};

// Naive implementation of exponential array (see https://www.youtube.com/watch?v=i-h95QIGchY)
template <typename T> class XArray {
private:
    struct XArray_Meta {
        uint8 shift;          // Exponent for size of first chunk
        uint8 chunk_count;
        uint32 element_size;
    } meta;

    struct XArray_Header {
        uint64 n;
        uint8* chunks[];
    }* header;

    enum {
        MAX_XARRAY_CHUNKS = 32
    };

    void* Get(uint64 i) {
        uint32 chunk_index = 0;
        uint64 chunk_size = 1 << meta.shift;
        uint64 element = i;

        uint64 shift_count = i >> meta.shift;
        if (shift_count > 0) {
            chunk_index = MSB64(shift_count);
            element -= chunk_size << chunk_index;
            chunk_index++;
        }

        return header->chunks[chunk_index] + element * meta.element_size; 
    }

    void NewChunk(memory_size size) {
        if (meta.chunk_count >= MAX_XARRAY_CHUNKS) {
            Assert(false, "Xarray chunk index overflow.");
        }
        header->chunks[meta.chunk_count++] = (uint8*)malloc(size);
    }

public:
    XArray() {
        meta = { 4, 0, sizeof(T) };
        header = (XArray_Header*)calloc(MAX_XARRAY_CHUNKS + 1, sizeof(void*));
        NewChunk(meta.element_size * (1 << meta.shift));
    }

    ~XArray() {
        for (int i = 0; i < meta.chunk_count; i++) {
            free(header->chunks[i]);
        }

        free(header);
    }

    const T& operator[] (uint64 i) const {
        Assert(0 <= i && i < header->n, "Index out of range.");
        return *(T*)Get(i);
    }

    T* Insert(const T& element = {}) {
        uint64 total_size = meta.element_size * (1 << (meta.shift + meta.chunk_count - 1));
        uint64 new_index = header->n++;
        if (header->n * sizeof(T) >= total_size) {
            NewChunk(total_size);
        }

        T* pointer = (T*)Get(new_index);
        *pointer = element;
        return pointer;
    }

    uint64 Size() {
        return header->n;
    }

    void Clear() {
        header->n = 0;
    }
};

template <typename S, typename T> class Hash_Table {
private:
    memory_size mask;
    memory_size used;
    struct Entry {
        uint64 hash;
        S key;
        T value;
    }* entries;
    bool* occupied;

    uint64 Hash(S key) {
        return XXH64(&key, sizeof(S), 0);
    }

    void Resize() {
        memory_size size = mask + 1;
        memory_size new_size = size << 1;
        memory_size new_mask = new_size - 1;
        void* new_memory = calloc(new_size, sizeof(bool) + sizeof(Entry));
        Entry* new_entries = (Entry*)new_memory;
        bool* new_occupied = (bool*)(new_entries + new_size);

        for (memory_size i = 0; i < size; i++) {
            if (occupied[i]) {
                Entry* entry = entries + i;
                memory_size new_index = entry->hash & new_mask;
    
                while (new_occupied[new_index]) {
                    new_index = (new_index + 1) & new_mask;
                }
                
                Entry* new_entry = new_entries + new_index;
                *new_entry = *entry;
                new_occupied[new_index] = true;
            }
        }

        free(entries);
        entries = new_entries;
        occupied = new_occupied;
        mask = new_mask;
    }

    memory_size Find(uint64 hash, S key, bool& found) {
        memory_size index = hash & mask;

        memory_size test_index = index;
        while (occupied[test_index]) {
            Entry* entry = entries + test_index;
            if (entry->hash == hash && entry->key == key) {
                found = true;
                return test_index;
            }
            test_index = (test_index + 1) & mask;
            if (test_index == index) break;
        }
        found = false;
        return test_index;
    }

public:
    Hash_Table(uint32 size_exponent = 6) {
        used = 0;
        memory_size size = (uint64)1 << size_exponent;
        mask = size - 1;
        void* memory = calloc(size, sizeof(bool) + sizeof(Entry));
        entries = (Entry*)memory;
        occupied = (bool*)(entries + size);
    }

    ~Hash_Table() {
        free(entries);
        used = 0;
        mask = 0;
        entries = nullptr;
        occupied = nullptr;
    }

    void Insert(S key, T value) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        Entry* entry = entries + index;
        if (!found) {
            used += 1;
            // Resize if 50% full
            if (2 * used >= mask + 1) {
                Resize();
                index = Find(hash, key, found);
                entry = entries + index;
            }
            occupied[index] = true;
            entry->hash = hash;
            entry->key = key;
        }
        entry->value = value;
    }

    void Delete(S key) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        if (!found) return;

        used -= 1;
        memory_size next_index = index;
        while (true) {
            next_index = (next_index + 1) & mask;
            if (!occupied[next_index]) break;
            Entry* next_entry = entries + next_index;
            memory_size ideal_index = next_entry->hash & mask;
            bool stays = (index <= next_index) ?
                (index < ideal_index && ideal_index <= next_index) : 
                (index < ideal_index || ideal_index <= next_index);
            if (stays) continue;
            entries[index] = entries[next_index];
            index = next_index;
        }
        occupied[index] = false;
    }

    bool Get(S key, T** value) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        if (found) *value = &entries[index].value;
        return found;
    }
};

template <typename T> class Hash_Table<String, T> {
private:
    memory_size mask;
    memory_size used;
    struct Entry {
        uint64 hash;
        String key;
        T value;
    }* entries;
    bool* occupied;

    uint64 Hash(String key) {
        return XXH64(key.content, key.length, 0);
    }

    void Resize() {
        memory_size size = mask + 1;
        memory_size new_size = size << 1;
        memory_size new_mask = new_size - 1;
        void* new_memory = calloc(new_size, sizeof(bool) + sizeof(Entry));
        Entry* new_entries = (Entry*)new_memory;
        bool* new_occupied = (bool*)(new_entries + new_size);

        for (memory_size i = 0; i < size; i++) {
            if (occupied[i]) {
                Entry* entry = entries + i;
                memory_size new_index = entry->hash & new_mask;
    
                while (new_occupied[new_index]) {
                    new_index = (new_index + 1) & new_mask;
                }
                
                Entry* new_entry = new_entries + new_index;
                *new_entry = *entry;
                new_occupied[new_index] = true;
            }
        }

        free(entries);
        entries = new_entries;
        occupied = new_occupied;
        mask = new_mask;
    }

    memory_size Find(uint64 hash, String key, bool& found) {
        memory_size index = hash & mask;

        memory_size test_index = index;
        while (occupied[test_index]) {
            Entry* entry = entries + test_index;
            if (entry->hash == hash && entry->key == key) {
                found = true;
                return test_index;
            }
            test_index = (test_index + 1) & mask;
            if (test_index == index) break;
        }
        found = false;
        return test_index;
    }

public:
    Hash_Table(uint32 size_exponent = 6) {
        used = 0;
        memory_size size = (uint64)1 << size_exponent;
        mask = size - 1;
        void* memory = calloc(size, sizeof(bool) + sizeof(Entry));
        entries = (Entry*)memory;
        occupied = (bool*)(entries + size);
    }

    ~Hash_Table() {
        free(entries);
        used = 0;
        mask = 0;
        entries = nullptr;
        occupied = nullptr;
    }

    T* Insert(String key) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        Entry* entry = entries + index;
        if (!found) {
            used += 1;
            // Resize if 50% full
            if (2 * used >= mask + 1) {
                Resize();
                index = Find(hash, key, found);
                entry = entries + index;
            }
            occupied[index] = true;
            entry->hash = hash;
            entry->key = key;
        }
        return &entry->value;
    }

    void Delete(String key) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        if (!found) return;

        used -= 1;
        memory_size next_index = index;
        while (true) {
            next_index = (next_index + 1) & mask;
            if (!occupied[next_index]) break;
            Entry* next_entry = entries + next_index;
            memory_size ideal_index = next_entry->hash & mask;
            bool stays = (index <= next_index) ?
                (index < ideal_index && ideal_index <= next_index) : 
                (index < ideal_index || ideal_index <= next_index);
            if (stays) continue;
            entries[index] = entries[next_index];
            index = next_index;
        }
        occupied[index] = false;
    }

    bool Get(String key, T** value) {
        uint64 hash = Hash(key);
        bool found;
        memory_size index = Find(hash, key, found);
        if (found) *value = &entries[index].value;
        return found;
    }
};
