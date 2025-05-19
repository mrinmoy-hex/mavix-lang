//
// Created by mrinmoy on 5/19/25.
//

#ifndef VM_H
#define VM_H

#include  "chunk.h"

typedef struct {
    Chunk* chunk;       // takes an entire chunk of code
} VM;

void initVM();
void freeVM();


#endif //VM_H
