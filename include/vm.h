//
// Created by mrinmoy on 5/19/25.
//

#ifndef VM_H
#define VM_H

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;       // takes an entire chunk of code
    uint8_t* ip;        // Instruction pointer (points to the next instruction)
    Value stack[STACK_MAX];
    Value* stackTop;    // points to the next value 
} VM;


// VM responses
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;


void initVM();
void freeVM();

// main entrypoint of VM
InterpretResult interpret(const char* source);

// Stack protocol operation
/*
    Push a new value onto the top of the stack.
*/
void push(Value value );
/*
    Pop out the most recently push value back.
*/
Value pop();

#endif //VM_H
