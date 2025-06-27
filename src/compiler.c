//
// Created by mrinmoy on 6/7/25.
//

#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

/*
Working mechanism of compiler (Mavix)
- The compiler will take the user’s program and fill up the chunk with bytecode.
Then it sends the completed chunk over to the VM to be executed. 
*/
bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
}
