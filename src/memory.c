#include <stdlib.h>

#include "memory.h"

// Function for handling all dynamic memory management in "mavix"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    // handle edge cases
    if (result == NULL) exit(1);
    return result;
}