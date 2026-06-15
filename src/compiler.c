#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif


// ============================================================================
// DATA STRUCTURES & TYPE DEFINITIONS
// ============================================================================

/**
 * Parser structure
 * Tracks parsing state and error management flags
 */
typedef struct {
    Token current;              // The token currently being examined
    Token previous;             // The last token that was consumed
    bool hadError;              // Set true globally if any code compilation error occurs
    bool panicMode;             // Set true during an error cascade to suppress spam errors
} Parser;


/**
 * Operator precedence levels
 * Used to determine evaluation order in expressions
 * Higher values = tighter binding = evaluated first
 */
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,            // =
    PREC_OR,                    // or
    PREC_AND,                   // and
    PREC_EQUALITY,              // == !=
    PREC_COMPARISON,            // < > <= >=
    PREC_TERM,                  // + -
    PREC_FACTOR,                // * /
    PREC_UNARY,                 // ! -
    PREC_CALL,                  // . ()
    PREC_PRIMARY
} Precedence;


/**
 * Function pointer type for parsing functions
 * Used in the ParseRule lookup table
 */
typedef void (*ParseFn)();


/**
 * Parse rule structure
 * Defines how to handle a particular token type in different contexts
 */
typedef struct {
    ParseFn prefix;             // Function to call when token appears at start of expression
    ParseFn infix;              // Function to call when token appears between expressions
    Precedence precedence;      // Precedence level for infix operators
} ParseRule;


// ============================================================================
// GLOBAL STATE
// ============================================================================

Parser parser;                  // Global parser state
Chunk* compilingChunk;          // The active bytecode array being filled by the compiler


// ============================================================================
// UTILITY FUNCTIONS - CHUNK ACCESS
// ============================================================================

/**
 * Returns the active chunk being compiled
 * Encapsulated to seamlessly support nested functions later
 */
static Chunk* currentChunk() {
    return compilingChunk;
}


// ============================================================================
// ERROR REPORTING & RECOVERY
// ============================================================================

/**
 * Low-level diagnostic reporter
 * Formats and prints syntax errors to stderr with context information
 * 
 * @param token  The token where the error occurred
 * @param message  Description of the error
 */
static void errorAt(Token* token, const char* message) {
    // Suppress cascade errors until syntax syncs up
    if (parser.panicMode) {
        return;
    }

    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR) {
        // Nothing; the error token's raw text contains the error message string
    }
    else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}


/**
 * Reports an error on the previously consumed token
 * 
 * @param message  Description of the error
 */
static void error(const char* message) {
    errorAt(&parser.previous, message);
}


/**
 * Reports an error on the current (upcoming) token
 * 
 * @param message  Description of the error
 */
static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}


// ============================================================================
// LEXICAL SCANNING & TOKEN CONSUMPTION
// ============================================================================

/**
 * Steps forward through the token stream
 * Automatically intercepts and reports scanner errors
 */
static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) {
            break;
        }

        errorAtCurrent(parser.current.start);
    }
}


/**
 * Asserts the next token matches 'type'
 * If true, advances past it; otherwise triggers a parsing error
 * 
 * @param type  The expected token type
 * @param message  Error message if token doesn't match
 */
static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}


// ============================================================================
// BYTECODE EMISSION & CODE GENERATION
// ============================================================================

/**
 * Appends a single byte instruction to the active chunk
 * Tracked to the previous token's line for error reporting
 * 
 * @param byte  The byte to write
 */
static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}


/**
 * Utility to write an opcode and its immediate operand byte consecutively
 * 
 * @param byte1  The opcode
 * @param byte2  The operand
 */
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}


/**
 * Emits the terminating bytecode return signal
 * Placed at the end of compiled code
 */
static void emitReturn() {
    emitByte(OP_RETURN);
}


/**
 * Creates a constant value and returns its index
 * The index is stored as an operand to OP_CONSTANT
 * 
 * @param value  The value to add as a constant
 * @return  The index of the constant, or 0 if too many constants
 */
static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}


/**
 * Emits an OP_CONSTANT instruction with a constant value
 * 
 * @param value  The value to emit as a constant
 */
static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}


/**
 * Wraps up compilation tasks once code processing is complete
 * Emits the return opcode and optionally disassembles for debugging
 */
static void endCompiler() {
    emitReturn();

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}


// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);


// ============================================================================
// EXPRESSION PARSING - PREFIX, INFIX, AND OPERATORS
// ============================================================================

/**
 * Parses binary operators (arithmetic and comparison)
 * Handles: + - * /
 * Uses precedence climbing to properly order operations
 */
static void binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);

    // Parse the right operand with higher precedence
    parsePrecedence((Precedence)(rule->precedence + 1));

    // Emit the appropriate opcode for the operator
    switch (operatorType) {
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;

        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;

        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;

        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;

        default:
            return;  // Unreachable
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


/**
 * Parses grouped expressions enclosed in parentheses
 * Handles: ( expression )
 */
static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}


/**
 * Parses numeric literals
 * Converts the token's string representation to a double value
 */
static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}


/**
 * Parses unary operators
 * Handles: - (negation)
 * Recursively parses the operand with unary precedence
 */
static void unary() {
    TokenType operatorType = parser.previous.type;

    // Compile the operand with unary precedence
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction
    switch (operatorType) {
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;

        default:
            return;  // Unreachable
    }
}


// ============================================================================
// PARSING RULE TABLE
// ============================================================================

/**
 * Parse rules array indexed by TokenType
 * Defines how to parse each token type as a prefix, infix, or both
 * Example: TOKEN_MINUS can be unary (prefix) or binary (infix)
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


// ============================================================================
// CORE PARSING ALGORITHM - PRATT PARSING
// ============================================================================

/**
 * Parses expressions using Pratt's precedence climbing algorithm
 * 
 * The key insight: prefix parsing handles the initial value, then infix
 * operators are parsed while their precedence is >= the current minimum
 * 
 * @param precedence  The minimum operator precedence to continue parsing
 */
static void parsePrecedence(Precedence precedence) {
    // Get the next token and look up its prefix rule
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;

    // A valid expression must start with a prefix operator or primary value
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    // Execute the prefix rule (e.g., number(), grouping(), unary())
    prefixRule();

    // Continue parsing infix operators while their precedence is high enough
    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}


/**
 * Retrieves the parse rule for a given token type
 * 
 * @param type  The token type to look up
 * @return  Pointer to the ParseRule for this token type
 */
static ParseRule* getRule(TokenType type) {
    return &rules[type];
}


/**
 * Parses an expression at the assignment precedence level
 * This is the entry point for expression parsing
 */
static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}


// ============================================================================
// MAIN COMPILATION ENTRY POINT
// ============================================================================

/**
 * Orchestrates the entire compilation process
 * Parses source code string and populates the given bytecode chunk
 * 
 * @param source  Pointer to the source code string to compile
 * @param chunk  Pointer to the Chunk where bytecode will be written
 * @return  true if compilation succeeded, false if syntax errors occurred
 */
bool compile(const char* source, Chunk* chunk) {
    // Initialize the scanner with the source code
    initScanner(source);
    compilingChunk = chunk;

    // Reset error tracking state
    parser.hadError = false;
    parser.panicMode = false;

    // Prime the pump by fetching the first valid token
    advance();

    // Parse the main expression
    expression();

    // Ensure the entire input was consumed
    consume(TOKEN_EOF, "Expect end of expression.");

    // Finalize bytecode and emit debugging info if enabled
    endCompiler();

    // Return success/failure based on whether any errors occurred
    return !parser.hadError;
}