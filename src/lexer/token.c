#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Keyword lookup table
typedef struct {
    const char* keyword;
    TokenType type;
} KeywordInfo;

static const KeywordInfo keywords[] = {
    {"int", TOKEN_INT},
    {"float", TOKEN_FLOAT},
    {"char", TOKEN_CHAR},
    {"bool", TOKEN_BOOL},
    {"void", TOKEN_VOID},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"return", TOKEN_RETURN},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"null", TOKEN_NULL},
    {NULL, TOKEN_UNKNOWN}  // Sentinel
};

Token* token_create(TokenType type, const char* lexeme, int line, int column) {
    Token* token = malloc(sizeof(Token));
    if (token == NULL) return NULL;

    token->type = type;
    token->lexeme = strdup_safe(lexeme);
    token->line = line;
    token->column = column;

    // Initialize literal values
    memset(&token->literal, 0, sizeof(token->literal));

    return token;
}

Token* token_create_with_literal(TokenType type, const char* lexeme, int line, int column) {
    Token* token = token_create(type, lexeme, line, column);
    if (token == NULL) return NULL;

    // Parse literal values based on token type
    switch (type) {
        case TOKEN_INTEGER_LITERAL:
            if (lexeme != NULL) {
                token->literal.int_value = atoi(lexeme);
            }
            break;

        case TOKEN_FLOAT_LITERAL:
            if (lexeme != NULL) {
                token->literal.float_value = (float)atof(lexeme);
            }
            break;

        case TOKEN_STRING_LITERAL:
            if (lexeme != NULL && strlen(lexeme) >= 2) {
                // Remove quotes from string literal
                size_t len = strlen(lexeme) - 2;
                token->literal.string_value = malloc(len + 1);
                if (token->literal.string_value != NULL) {
                    strncpy(token->literal.string_value, lexeme + 1, len);
                    token->literal.string_value[len] = '\0';
                }
            }
            break;

        case TOKEN_CHAR_LITERAL:
            if (lexeme != NULL && strlen(lexeme) >= 3) {
                // Remove quotes from char literal
                token->literal.char_value = lexeme[1];
            }
            break;

        default:
            // For non-literal tokens, literal values remain zero/NULL
            break;
    }

    return token;
}

void token_free(Token* token) {
    if (token == NULL) return;

    free(token->lexeme);

    // Free string literal if present
    if (token->type == TOKEN_STRING_LITERAL && token->literal.string_value != NULL) {
        free(token->literal.string_value);
    }

    free(token);
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_INT: return "INT";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_BOOL: return "BOOL";
        case TOKEN_VOID: return "VOID";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_BREAK: return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_NULL: return "NULL";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INTEGER_LITERAL: return "INTEGER_LITERAL";
        case TOKEN_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";
        case TOKEN_CHAR_LITERAL: return "CHAR_LITERAL";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MULTIPLY: return "MULTIPLY";
        case TOKEN_DIVIDE: return "DIVIDE";
        case TOKEN_MODULO: return "MODULO";
        case TOKEN_INCREMENT: return "INCREMENT";
        case TOKEN_DECREMENT: return "DECREMENT";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_NOT_EQUAL: return "NOT_EQUAL";
        case TOKEN_LESS: return "LESS";
        case TOKEN_GREATER: return "GREATER";
        case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
        case TOKEN_LOGICAL_AND: return "LOGICAL_AND";
        case TOKEN_LOGICAL_OR: return "LOGICAL_OR";
        case TOKEN_LOGICAL_NOT: return "LOGICAL_NOT";
        case TOKEN_BITWISE_AND: return "BITWISE_AND";
        case TOKEN_BITWISE_OR: return "BITWISE_OR";
        case TOKEN_BITWISE_XOR: return "BITWISE_XOR";
        case TOKEN_BITWISE_NOT: return "BITWISE_NOT";
        case TOKEN_LEFT_SHIFT: return "LEFT_SHIFT";
        case TOKEN_RIGHT_SHIFT: return "RIGHT_SHIFT";
        case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
        case TOKEN_LEFT_BRACKET: return "LEFT_BRACKET";
        case TOKEN_RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_DOT: return "DOT";
        case TOKEN_COLON: return "COLON";
        case TOKEN_QUESTION: return "QUESTION";
        case TOKEN_UNKNOWN: return "UNKNOWN";
        case TOKEN_NEWLINE: return "NEWLINE";
        default: return "UNKNOWN";
    }
}

bool token_is_keyword(const char* lexeme, TokenType* type) {
    if (lexeme == NULL || type == NULL) return false;

    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (strcmp(lexeme, keywords[i].keyword) == 0) {
            *type = keywords[i].type;
            return true;
        }
    }

    return false;
}

void token_print(Token* token) {
    if (token == NULL) {
        printf("Token: NULL\n");
        return;
    }

    printf("Token: %s", token_type_to_string(token->type));

    if (token->lexeme != NULL && strlen(token->lexeme) > 0) {
        printf(" ('%s')", token->lexeme);
    }

    printf(" at %d:%d", token->line, token->column);

    // Print literal values if applicable
    switch (token->type) {
        case TOKEN_INTEGER_LITERAL:
            printf(" value=%d", token->literal.int_value);
            break;
        case TOKEN_FLOAT_LITERAL:
            printf(" value=%f", token->literal.float_value);
            break;
        case TOKEN_STRING_LITERAL:
            if (token->literal.string_value != NULL) {
                printf(" value=\"%s\"", token->literal.string_value);
            }
            break;
        case TOKEN_CHAR_LITERAL:
            printf(" value='%c'", token->literal.char_value);
            break;
        default:
            break;
    }

    printf("\n");
}