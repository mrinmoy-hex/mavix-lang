#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ANSI Color Escape Codes
#define COLOR_RESET   "\x1b[0m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_GRAY    "\x1b[90m"

// Read Eval Print Loop
static void repl() {
    // clear the screen and show a banner
    printf("\033[H\033[J"); // ANSI escape code to clear screen
    printf(COLOR_CYAN "mavix bytecode interpreter (Version 1.0.0)\n" COLOR_RESET);
    printf(COLOR_GRAY "Type mavix code below. Press Ctrl+D (or Ctrl+Z on Windows) to exit.\n\n" COLOR_RESET);

    char line[1024];
    for (;;) {
        printf(COLOR_GREEN ">>> " COLOR_RESET);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\nGoodbye!\n");
            break;
        }

        // skip empty lines / accidental enters
        line[strcspn(line, "\n")] = '\0';

        // if the line is empty after stripping
        if (strlen(line) == 0) {
            continue;
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
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
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