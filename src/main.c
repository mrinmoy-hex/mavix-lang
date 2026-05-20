#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Read Eval Print Loop
static void repl() {
    char line[1024];
    for (;;) {
        printf("\x1b[1;36m›\x1b[0m ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static char* readFile(const char* path) {
    // Open file in read-bin mode
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\" .\n", path);
        exit(74);       // standard I/O error
    }

    // seek to the end to measure the total file size
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);   // reset pointer back to beginnning

    char *buffer = (char*) malloc(fileSize + 1);    // +1 for null terminator
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n");
        exit(74);
    }

    // stream file data into the allocated RAM buffer
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    char *source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR)  exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)  exit(70);
}


int main(int argc, const char* argv[]) {

    initVM();

    if (argc == 1) {
        repl();
    }
    else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: mavix [path]\n");
        exit(64);
    }

    freeVM();

    return 0;
}