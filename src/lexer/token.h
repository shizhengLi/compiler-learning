#ifndef TOKEN_H
#define TOKEN_H

#include "../common/common.h"

// Token types
typedef enum {
    // End of file
    TOKEN_EOF = 0,

    // Keywords
    TOKEN_INT = 1,
    TOKEN_FLOAT,
    TOKEN_CHAR,
    TOKEN_BOOL,
    TOKEN_VOID,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,

    // Identifiers and literals
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_CHAR_LITERAL,

    // Operators
    TOKEN_ASSIGN,       // =
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_MULTIPLY,     // *
    TOKEN_DIVIDE,       // /
    TOKEN_MODULO,       // %
    TOKEN_INCREMENT,    // ++
    TOKEN_DECREMENT,    // --

    // Comparison operators
    TOKEN_EQUAL,        // ==
    TOKEN_NOT_EQUAL,    // !=
    TOKEN_LESS,         // <
    TOKEN_GREATER,      // >
    TOKEN_LESS_EQUAL,   // <=
    TOKEN_GREATER_EQUAL,// >=

    // Logical operators
    TOKEN_LOGICAL_AND,  // &&
    TOKEN_LOGICAL_OR,   // ||
    TOKEN_LOGICAL_NOT,  // !

    // Bitwise operators
    TOKEN_BITWISE_AND,  // &
    TOKEN_BITWISE_OR,   // |
    TOKEN_BITWISE_XOR,  // ^
    TOKEN_BITWISE_NOT,  // ~
    TOKEN_LEFT_SHIFT,   // <<
    TOKEN_RIGHT_SHIFT,  // >>

    // Delimiters
    TOKEN_LEFT_PAREN,   // (
    TOKEN_RIGHT_PAREN,  // )
    TOKEN_LEFT_BRACE,   // {
    TOKEN_RIGHT_BRACE,  // }
    TOKEN_LEFT_BRACKET, // [
    TOKEN_RIGHT_BRACKET,// ]
    TOKEN_SEMICOLON,    // ;
    TOKEN_COMMA,        // ,
    TOKEN_DOT,          // .
    TOKEN_COLON,        // :
    TOKEN_QUESTION,     // ?

    // Special tokens
    TOKEN_UNKNOWN,
    TOKEN_NEWLINE
} TokenType;

// Token structure
typedef struct Token {
    TokenType type;
    char* lexeme;        // The actual text
    int line;            // Line number (1-based)
    int column;          // Column number (1-based)
    union {
        int int_value;
        float float_value;
        char* string_value;
        char char_value;
    } literal;
} Token;

// Token functions
Token* token_create(TokenType type, const char* lexeme, int line, int column);
Token* token_create_with_literal(TokenType type, const char* lexeme, int line, int column);
void token_free(Token* token);
const char* token_type_to_string(TokenType type);
bool token_is_keyword(const char* lexeme, TokenType* type);
void token_print(Token* token);

#endif // TOKEN_H