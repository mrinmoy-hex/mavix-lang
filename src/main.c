#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "vm.h"

/**
 * @brief Starts the Read-Eval-Print Loop (REPL) for the Mavix language interpreter.
 * 
 * This function initializes and runs the REPL, allowing users to interactively
 * enter and evaluate Mavix language expressions. It continuously reads input
 * from the user, evaluates it, and prints the result until the user decides to exit.
 */
static void repl() {
    char line[1024];    // buffer to hold user input

    printf("Mavix v0.1 [REPL mode]\n");
    printf("Type 'exit' or press Ctrl+D to quit.\n");

    for (;;) {
        printf(">>> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;      // handle Ctrl+D (EOF)
        }

        interpret(line);
    }
}

/**
 * @brief Reads the contents of a file and returns it as a string.
 *
 * This function opens the file specified by the given path, reads its contents,
 * and returns the contents as a dynamically allocated string. The caller is
 * responsible for freeing the allocated memory.
 *
 * @param path The path to the file to be read.
 * @return A pointer to the dynamically allocated string containing the file contents,
 *         or NULL if an error occurs (e.g., the file cannot be opened).
 */
static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    
    // Handle failure to open the file
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    // Move to the end of the file to determine its size
    fseek(file, 0L, SEEK_END);          
    size_t fileSize = ftell(file);      // Get current position as the size
    rewind(file);                       // Go back to the beginning for reading

    // Allocate memory for the file content (+1 for null terminator)
    char* buffer = (char*) malloc(fileSize + 1);

    // Handle memory allocation failure
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    // Read the entire file into the buffer
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    // Handle file reading failure
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    // Null-terminate the string to safely treat it as a C string
    buffer[bytesRead] = '\0';

    fclose(file);

    return buffer;
}



static void runFile(const char* path) {
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    // handle edge cases
    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}


int main(int argc, const char* argv[]) {
    initVM();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: %s [script]\n", argv[0]);
        fprintf(stderr, "Run without arguments to enter interactive mode (REPL).\n");
        exit(64);
    }

    freeVM();
    return 0;
}
