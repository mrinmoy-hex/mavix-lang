//
// Created by mrinmoy on 6/7/25.
//

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif


typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

// Enum struct of Precedence table (from lower to higher)
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,    // =
    PREC_OR,            // or
    PREC_AND,           // and
    PREC_EQUALITY,      // == !=
    PREC_COMPARISION,   // < > <= >=
    PREC_TERM,          // + -
    PREC_FACTOR,        // * /
    PREC_UNARY,         // ! -
    PREC_CALL,          // . ()
    PREC_PRIMARY
} Precedence;


typedef void (*ParseFn)();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
/**
 * @brief Structure representing a parsing rule.
 */
} ParseRule;


Parser parser;

Chunk* compilingChunk;

static Chunk* currentChunk() {
    return compilingChunk;
}


// Reports a syntax error at the given token with the provided message
static void errorAt(Token* token, const char* message) {
    // if panic mode is active, return immediately
    if (parser.panicMode) return;

    // Enters panic mode to suppress further error messages 
    // until the parser has recovered (via synchronize()).
    parser.panicMode = true;

    // Print the error message with line number
    fprintf(stderr, "[line %d] Error ", token->line);

    if (token->type == TOKEN_EOF) {
        // If at end of file, indicate that.
        fprintf(stderr, "at end");
    } else if (token->type == TOKEN_ERROR) {
        // If it's already an error token (like from the scanner),
        // don't print any more context.
        // The message itself already contains the error detail.
    } else {
        // Otherwise, print the exact token where the error occurred.
        fprintf(stderr, "at '%.*s'", token->length, token->start);
    }

    // Set a flag indicating that an error has occurred.
    // This is used to prevent code generation or running if there were syntax issues.
    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

// Reports an error at the previous token (typically used when a rule fails after consuming a token).
static void error(const char* message) {
    errorAt(&parser.previous, message);
}

// Reports an error at the current token (typically used when we detect an error before consuming a token).
static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}


/**
 * @brief Advances the current position in the input stream.
 *
 * This function is responsible for moving the current position
 * forward in the input stream.
 */
static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();

        // Break the loop if we successfully scanned a valid token
        if (parser.current.type != TOKEN_ERROR) break;

        // Report error for invalid token and continue scanning.
        // The start of the token itself is reused as the error message in this case.
        errorAtCurrent(parser.current.start);
    }
}


/**
 * @brief Consumes a token of the specified type.
 *
 * This function checks if the current token matches the expected type.
 * If it does, the token is consumed. If it does not, an error message
 * is reported.
 *
 * @param type The expected type of the token to consume.
 * @param message The error message to report if the token type does not match.
 */
static void consume(TokenType type, const char* message) {

    // If the token matches the expected type, advance to the next token.
    if (parser.current.type == type) {
        advance();
        return;
    }

    // Otherwise, report an error at the unexpected token.
    // This will also trigger panic mode to avoid further errors.
    errorAtCurrent(message);
}


/*
#####################################
Emitting Bytecode
#####################################
*/

// Emits a single byte (usually an opcode or operand) into the current Chunk.
// Associates it with the source line for debugging and error reporting.
static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}


// Emits two bytes in sequence. Used for opcodes that require operands.
// Example: OP_CONSTANT <index>
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);    // Usually the opcode
    emitByte(byte2);    // Usually the operand (like index into constant table)
}


// Emits the OP_RETURN instruction (tells the VM to return from the function).
static void emitReturn() {
    emitByte(OP_RETURN);
}


// Adds value to the constant table
static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);

    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}


static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}


// Called at the end of compilation to finish the function.
// Emits a return instruction so the VM knows when to stop executing.
static void endCompiler() {
    emitReturn();

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}


/*
#############################
Parsing expressions
#############################
*/


static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);


// infix parser for binary operations
static void binary() {
    TokenType operatorType = parser.previous.type;

    // get the parsing rule to find precedence level of this operation
    ParseRule* rule = getRule(operatorType);

    // parse the right-hand operand with higher precedence (to bind tightly)
    parsePrecedence((Precedence)(rule->precedence + 1));

    // Emit the corresponding bytecode instruction for the binary operator
    switch (operatorType) {
        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
        default: return; // Unreachable.
    }
}



static void literal() {
    switch (parser.previous.type) {
        case TOKEN_FALSE:   emitByte(OP_FALSE); break;
        case TOKEN_NIL:     emitByte(OP_NIL); break;
        case TOKEN_TRUE:    emitByte(OP_TRUE); break;
        default: return;    // Unreachable.
    }
}


// compiling groupings
static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}


// compiling number literals
static void number() {

    double value = strtod(parser.previous.start, NULL);     // converts string into double value.
    emitConstant(NUMBER_VAL(value));
}


// compiling unary expression
static void unary() {
    TokenType operatorType = parser.previous.type;  // for the '-' part

    // Compile the operand.
    expression();

    // Emit the operator instruction.
    switch (operatorType) {
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return;    // Unreachable.
    }
}


/**
 * An array of ParseRule structures that define the parsing rules for the compiler.
 */
ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};




/**
 * @brief Parses expressions with the given precedence level.
 *
 * This function processes expressions in the source code that match or exceed
 * the specified precedence level. It is used to handle different levels of
 * operator precedence during the parsing phase of the compiler.
 *
 * @param precedence The precedence level to parse expressions for.
 */
static void parsePrecedence(Precedence precedence) {
    advance();      // read the next token and store it in parser.previous

    // Get the prefix parsing function for the current token
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");    // token isn't a valid start of an expression 
        return;
    }

    prefixRule();   // parse the prefix part (like number, variable, or grouping)

    // Keep parsing infix expression as long as their precedence is >= current level
    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();      // consume the infix operator
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();    // parse the infix operation (e.g. +, -. *. /)
    }
}




static ParseRule* getRule(TokenType type) {
    return &rules[type];
}


static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);   // lowest level precedence
}



/**
 * @brief Compiles the given source code into a chunk of bytecode.
 *
 * This function takes the source code as input and compiles it into a 
 * Chunk structure, which contains the bytecode representation of the 
 * source code. The compilation process involves lexical analysis, 
 * parsing, and code generation.
 *
 * @param source The source code to be compiled.
 * @param chunk A pointer to the Chunk structure where the compiled 
 *              bytecode will be stored.
 * @return true if the compilation was successful, false otherwise.
 */
bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk;     // Initializes the Chunk (for writing bytecode)

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();

    return !parser.hadError;
}
