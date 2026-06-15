#include <stdlib.h>

#include "memory.h"


/**
 * Central memory management function for the entire VM
 * Handles allocation, reallocation, and deallocation of memory
 * 
 * This function abstracts away direct calls to malloc/realloc/free,
 * making it easy to add memory tracking, garbage collection, or
 * custom allocation strategies in the future.
 * 
 * Behavior:
 *   - newSize == 0:  Free the allocation (deallocation)
 *   - oldSize > 0 && newSize > oldSize:  Expand existing allocation
 *   - oldSize > 0 && newSize < oldSize:  Shrink existing allocation
 *   - pointer == NULL && newSize > 0:  Allocate new memory
 * 
 * On allocation failure:
 *   - Prints error to stderr and exits immediately (exit code 1)
 *   - This prevents silent failures and corrupted state
 * 
 * @param pointer  Address of existing allocation, or NULL for new allocation
 * @param oldSize  Current size of the allocation (ignored by realloc, but
 *                 kept for potential future use in custom allocators)
 * @param newSize  Desired new size in bytes
 *                 If 0, the allocation is freed
 * @return  Pointer to the allocated memory, or NULL if newSize was 0
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    // Handle deallocation
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    // Attempt to allocate/reallocate memory to the new size
    // realloc() handles both allocation (if pointer is NULL) and resizing
    void* result = realloc(pointer, newSize);

    // Check for allocation failure
    // If realloc returns NULL, it means memory allocation failed
    if (result == NULL) {
        exit(1);
    }

    return result;
}