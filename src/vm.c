//
// Created by mrinmoy on 5/19/25.
//
#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"

#include <stdarg.h>
#include <stdio.h>

VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}

/**
 * @brief Reports a runtime error with a formatted message.
 *
 * This function prints a formatted error message to the standard error output,
 * using a format string and a variable number of arguments (similar to printf).
 * It is typically used to display errors that occur during the execution of the virtual machine.
 *
 * @param format The format string for the error message.
 * @param ...    Additional arguments to be formatted into the message.
 */
static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}


void initVM() {
    resetStack();
}

void freeVM() {

}

void push(Value value ) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}


/**
 * Executes the main interpreter loop for the virtual machine.
 *
 * This function runs the bytecode instructions loaded into the VM,
 * managing the instruction pointer, stack, and other VM state.
 * It processes instructions until a return or error condition is encountered.
 *
 * @return InterpretResult The result of the interpretation, indicating
 *         success, runtime error, or compile error.
 */
static InterpretResult run() {
    // helper macros
#define READ_BYTE() (*vm.ip++)      // reads the current byte and advances it
/* @note  
 * Uses READ_BYTE() to get the index of a constant in the constants array.
 *
 * @return:
 *  Returns the Value at that index. 
 * */
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)

    for (;;) {
    
    // For diagnostic logging 
/* 
When this flag is defined, the VM disassembles and prints each instruction right before 
executing it.
*/
#ifdef DEBUG_TRACE_EXECUTION
    // show the current content of the stack
    printf("          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        printf("[ ");
        printValue(*slot);
        printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int) (vm.ip - vm.chunk->code ));  // computes the current offset
#endif 

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_NIL: push(NIL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;

            case OP_ADD:        BINARY_OP(NUMBER_VAL, +); break;
            case OP_SUBTRACT:   BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:   BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:     BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;

            /*
            @note 
            The instruction needs a value to operate on, which it gets by popping from the 
            stack. It negates that, then pushes the result back on for later instructions to use.
            */
            case OP_NEGATE:     
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                } 
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;

            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}



/**
 * @brief Interprets the given source code.
 *
 * This function takes a string containing source code and executes it
 * using the virtual machine. It returns an InterpretResult indicating
 * the outcome of the interpretation, such as success or the type of error encountered.
 *
 * @param source A null-terminated string containing the source code to interpret.
 * @return InterpretResult The result of interpreting the source code.
 */
InterpretResult interpret(const char* source) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}
