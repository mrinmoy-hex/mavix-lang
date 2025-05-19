//
// Created by mrinmoy on 5/19/25.
//
#include "common.h"
#include "vm.h"

VM vm;

void initVM() {

}

void freeVM() {

}

static InterpretResult run() {

}


InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}