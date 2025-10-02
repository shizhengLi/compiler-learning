#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "../common/common.h"

// Lexer structure
typedef struct Lexer {
    const char* source;       // Source code string
    int position;            // Current position in source
    int line;                // Current line number (1-based)
    int column;              // Current column number (1-based)
    char current_char;       // Current character
    bool had_error;          // Error flag
    Error* last_error;       // Last error
} Lexer;

// Lexer functions
Lexer* lexer_create(const char* source);
void lexer_free(Lexer* lexer);

// Tokenization functions
Token* lexer_next_token(Lexer* lexer);
Token* lexer_peek_token(Lexer* lexer);

// Utility functions
static void advance(Lexer* lexer);
static char peek(Lexer* lexer);
static bool match(Lexer* lexer, char expected);
static void skip_whitespace(Lexer* lexer);
static void skip_comment(Lexer* lexer);
static Token* read_identifier(Lexer* lexer);
static Token* read_number(Lexer* lexer);
static Token* read_string(Lexer* lexer);
static Token* read_char(Lexer* lexer);
static Token* read_operator(Lexer* lexer);

// Error handling
Error* lexer_get_last_error(Lexer* lexer);
bool lexer_had_error(Lexer* lexer);
void lexer_clear_error(Lexer* lexer);

#endif // LEXER_H