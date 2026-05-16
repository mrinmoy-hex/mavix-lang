#ifndef mavix_vm_h
#define mavix_vm_h

#include "chunk.h"

typedef struct {
    Chunk *chunk;
} VM;

void initVM();
void freeVM();

#endif