#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"

// Doubles capacity, or sets to 8 if starting from zero
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

// Macro for resizing an array of a given type
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))


// Reallocates memory block to new size
void* reallocate(void* pointer, size_t oldSize, size_t newSize);


#endif