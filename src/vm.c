//
// Created by mrinmoy on 5/19/25.
//
#include "common.h"
#include "vm.h"

#include <stdio.h>

VM vm;

void initVM() {

}

void freeVM() {

}


static InterpretResult run() {
    // helper macros
#define READ_BYTE() (*vm.ip++)      // reads the current byte and advances it
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for (;;) {
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                printValue(constant);
                printf("\n");
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}



InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}