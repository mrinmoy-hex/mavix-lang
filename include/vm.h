//
// Created by mrinmoy on 5/19/25.
//

#ifndef VM_H
#define VM_H

#include  "chunk.h"

typedef struct {
    Chunk* chunk;       // takes an entire chunk of code
    uint8_t* ip;        // Instruction pointer (points to the next instruction)
} VM;


// VM response
typedef struct {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;


void initVM();
void freeVM();
// main entrypoint of VM
InterpretResult interpret(Chunk* chunk);

#endif //VM_H
