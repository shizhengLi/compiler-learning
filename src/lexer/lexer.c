#include "lexer.h"

Lexer* lexer_create(const char* source) {
    Lexer* lexer = malloc(sizeof(Lexer));
    if (lexer == NULL) return NULL;

    lexer->source = strdup_safe(source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = (lexer->source != NULL) ? lexer->source[0] : '\0';
    lexer->had_error = false;
    lexer->last_error = NULL;

    return lexer;
}

void lexer_free(Lexer* lexer) {
    if (lexer == NULL) return;

    free((char*)lexer->source);
    error_free(lexer->last_error);
    free(lexer);
}

Token* lexer_next_token(Lexer* lexer) {
    if (lexer == NULL || lexer->source == NULL || lexer->current_char == '\0') {
        return token_create(TOKEN_EOF, "", lexer->line, lexer->column);
    }

    // Skip whitespace but not newlines
    skip_whitespace(lexer);

    // Check for newline token
    if (lexer->current_char == '\n') {
        int line = lexer->line;
        int column = lexer->column;
        advance(lexer);
        return token_create(TOKEN_NEWLINE, "\n", line, column);
    }

    if (lexer->current_char == '\0') {
        return token_create(TOKEN_EOF, "", lexer->line, lexer->column);
    }

    // Check for identifiers and keywords
    if (isalpha(lexer->current_char) || lexer->current_char == '_') {
        return read_identifier(lexer);
    }

    // Check for numbers
    if (isdigit(lexer->current_char)) {
        return read_number(lexer);
    }

    // Check for string literals
    if (lexer->current_char == '"') {
        return read_string(lexer);
    }

    // Check for character literals
    if (lexer->current_char == '\'') {
        return read_char(lexer);
    }

    // Check for operators and delimiters
    return read_operator(lexer);
}

Token* lexer_peek_token(Lexer* lexer) {
    // Save current state
    int saved_position = lexer->position;
    int saved_line = lexer->line;
    int saved_column = lexer->column;
    char saved_char = lexer->current_char;

    // Get next token
    Token* token = lexer_next_token(lexer);

    // Restore state
    lexer->position = saved_position;
    lexer->line = saved_line;
    lexer->column = saved_column;
    lexer->current_char = saved_char;

    return token;
}

Error* lexer_get_last_error(Lexer* lexer) {
    return lexer ? lexer->last_error : NULL;
}

bool lexer_had_error(Lexer* lexer) {
    return lexer ? lexer->had_error : false;
}

void lexer_clear_error(Lexer* lexer) {
    if (lexer) {
        lexer->had_error = false;
        error_free(lexer->last_error);
        lexer->last_error = NULL;
    }
}

// Utility functions (static)
static void advance(Lexer* lexer) {
    if (lexer == NULL || lexer->source == NULL) return;

    if (lexer->current_char == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }

    lexer->position++;
    lexer->current_char = lexer->source[lexer->position];
}

static char peek(Lexer* lexer) {
    if (lexer == NULL || lexer->source == NULL) return '\0';
    return lexer->source[lexer->position + 1];
}

static bool match(Lexer* lexer, char expected) {
    if (lexer == NULL || lexer->source == NULL) return false;

    if (lexer->current_char == expected) {
        advance(lexer);
        return true;
    }
    return false;
}

static void skip_whitespace(Lexer* lexer) {
    while (lexer != NULL && lexer->source != NULL && lexer->current_char != '\0') {
        if (lexer->current_char == ' ' || lexer->current_char == '\t' ||
            lexer->current_char == '\r') {
            advance(lexer);
        } else {
            break;
        }
    }
}

static void skip_comment(Lexer* lexer) {
    // Will be implemented
}

static Token* read_identifier(Lexer* lexer) {
    if (lexer == NULL || lexer->source == NULL) return NULL;

    int start_line = lexer->line;
    int start_column = lexer->column;

    StringBuffer* buffer = string_buffer_create(32);
    while (lexer->current_char != '\0' &&
           (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        string_buffer_append_char(buffer, lexer->current_char);
        advance(lexer);
    }

    char* lexeme = strdup(buffer->data);
    string_buffer_free(buffer);

    // Check if it's a keyword
    TokenType type = TOKEN_IDENTIFIER;
    token_is_keyword(lexeme, &type);

    Token* token = token_create(type, lexeme, start_line, start_column);
    free(lexeme);
    return token;
}

static Token* read_number(Lexer* lexer) {
    if (lexer == NULL || lexer->source == NULL) return NULL;

    int start_line = lexer->line;
    int start_column = lexer->column;

    StringBuffer* buffer = string_buffer_create(32);
    bool has_decimal = false;

    while (lexer->current_char != '\0') {
        if (isdigit(lexer->current_char)) {
            string_buffer_append_char(buffer, lexer->current_char);
            advance(lexer);
        } else if (lexer->current_char == '.' && !has_decimal) {
            string_buffer_append_char(buffer, lexer->current_char);
            has_decimal = true;
            advance(lexer);
        } else {
            break;
        }
    }

    char* lexeme = strdup(buffer->data);
    string_buffer_free(buffer);

    TokenType type = has_decimal ? TOKEN_FLOAT_LITERAL : TOKEN_INTEGER_LITERAL;
    Token* token = token_create_with_literal(type, lexeme, start_line, start_column);
    free(lexeme);
    return token;
}

static Token* read_string(Lexer* lexer) {
    if (lexer == NULL || lexer->source == NULL) return NULL;

    int start_line = lexer->line;
    int start_column = lexer->column;

    advance(lexer); // consume opening quote

    StringBuffer* buffer = string_buffer_create(32);
    while (lexer->current_char != '\0' && lexer->current_char != '"') {
        if (lexer->current_char == '\\') {
            advance(lexer); // consume backslash
            if (lexer->current_char != '\0') {
                // Simple escape sequence handling
                switch (lexer->current_char) {
                    case 'n': string_buffer_append_char(buffer, '\n'); break;
                    case 't': string_buffer_append_char(buffer, '\t'); break;
                    case 'r': string_buffer_append_char(buffer, '\r'); break;
                    case '\\': string_buffer_append_char(buffer, '\\'); break;
                    case '"': string_buffer_append_char(buffer, '"'); break;
                    default: string_buffer_append_char(buffer, lexer->current_char); break;
                }
            }
        } else {
            string_buffer_append_char(buffer, lexer->current_char);
        }
        advance(lexer);
    }

    if (lexer->current_char == '"') {
        advance(lexer); // consume closing quote
    } else {
        // Unterminated string
        lexer->had_error = true;
        lexer->last_error = error_create(ERROR_LEXICAL, "Unterminated string literal",
                                          start_line, start_column, NULL);
    }

    char* content = strdup(buffer->data);
    char* lexeme = malloc(strlen(content) + 3);
    sprintf(lexeme, "\"%s\"", content);

    Token* token = token_create_with_literal(TOKEN_STRING_LITERAL, lexeme,
                                           start_line, start_column);
    if (token != NULL) {
        token->literal.string_value = content;
    } else {
        free(content);
    }

    free(lexeme);
    string_buffer_free(buffer);
    return token;
}

static Token* read_char(Lexer* lexer) {
    if (lexer == NULL || lexer->source == NULL) return NULL;

    int start_line = lexer->line;
    int start_column = lexer->column;

    advance(lexer); // consume opening quote

    if (lexer->current_char == '\0') {
        // Unterminated character literal
        lexer->had_error = true;
        lexer->last_error = error_create(ERROR_LEXICAL, "Unterminated character literal",
                                          start_line, start_column, NULL);
        return token_create(TOKEN_UNKNOWN, "'", start_line, start_column);
    }

    char content = '\0';
    if (lexer->current_char == '\\') {
        advance(lexer); // consume backslash
        if (lexer->current_char != '\0') {
            // Simple escape sequence handling
            switch (lexer->current_char) {
                case 'n': content = '\n'; break;
                case 't': content = '\t'; break;
                case 'r': content = '\r'; break;
                case '\\': content = '\\'; break;
                case '\'': content = '\''; break;
                case '\"': content = '\"'; break;
                default: content = lexer->current_char; break;
            }
        }
    } else {
        content = lexer->current_char;
    }

    advance(lexer); // consume character content

    if (lexer->current_char == '\'') {
        advance(lexer); // consume closing quote
    } else {
        // Unterminated character literal
        lexer->had_error = true;
        lexer->last_error = error_create(ERROR_LEXICAL, "Unterminated character literal",
                                          start_line, start_column, NULL);
        return token_create(TOKEN_UNKNOWN, "'", start_line, start_column);
    }

    // Create lexeme "'c'" where c is the character
    char lexeme[4] = {'\'', content, '\'', '\0'};
    Token* token = token_create_with_literal(TOKEN_CHAR_LITERAL, lexeme, start_line, start_column);
    if (token != NULL) {
        token->literal.char_value = content;
    }

    return token;
}

static Token* read_operator(Lexer* lexer) {
    if (lexer == NULL || lexer->source == NULL) return NULL;

    int start_line = lexer->line;
    int start_column = lexer->column;
    char current = lexer->current_char;
    char next = peek(lexer);

    // Multi-character operators
    switch (current) {
        case '=':
            if (next == '=') {
                advance(lexer); // consume '='
                advance(lexer); // consume '='
                return token_create(TOKEN_EQUAL, "==", start_line, start_column);
            }
            advance(lexer);
            return token_create(TOKEN_ASSIGN, "=", start_line, start_column);

        case '!':
            if (next == '=') {
                advance(lexer); // consume '!'
                advance(lexer); // consume '='
                return token_create(TOKEN_NOT_EQUAL, "!=", start_line, start_column);
            }
            advance(lexer);
            return token_create(TOKEN_LOGICAL_NOT, "!", start_line, start_column);

        case '&':
            if (next == '&') {
                advance(lexer); // consume '&'
                advance(lexer); // consume '&'
                return token_create(TOKEN_LOGICAL_AND, "&&", start_line, start_column);
            }
            advance(lexer);
            return token_create(TOKEN_BITWISE_AND, "&", start_line, start_column);

        case '|':
            if (next == '|') {
                advance(lexer); // consume '|'
                advance(lexer); // consume '|'
                return token_create(TOKEN_LOGICAL_OR, "||", start_line, start_column);
            }
            advance(lexer);
            return token_create(TOKEN_BITWISE_OR, "|", start_line, start_column);

        case '^':
            advance(lexer);
            return token_create(TOKEN_BITWISE_XOR, "^", start_line, start_column);

        case '~':
            advance(lexer);
            return token_create(TOKEN_BITWISE_NOT, "~", start_line, start_column);

        case '+':
            if (next == '+') {
                advance(lexer); // consume '+'
                advance(lexer); // consume '+'
                return token_create(TOKEN_INCREMENT, "++", start_line, start_column);
            }
            advance(lexer);
            return token_create(TOKEN_PLUS, "+", start_line, start_column);

        case '-':
            if (next == '-') {
                advance(lexer); // consume '-'
                advance(lexer); // consume '-'
                return token_create(TOKEN_DECREMENT, "--", start_line, start_column);
            }
            advance(lexer);
            return token_create(TOKEN_MINUS, "-", start_line, start_column);

        case '<':
            if (next == '=') {
                advance(lexer); // consume '<'
                advance(lexer); // consume '='
                return token_create(TOKEN_LESS_EQUAL, "<=", start_line, start_column);
            } else if (next == '<') {
                advance(lexer); // consume '<'
                advance(lexer); // consume '<'
                return token_create(TOKEN_LEFT_SHIFT, "<<", start_line, start_column);
            }
            advance(lexer);
            return token_create(TOKEN_LESS, "<", start_line, start_column);

        case '>':
            if (next == '=') {
                advance(lexer); // consume '>'
                advance(lexer); // consume '='
                return token_create(TOKEN_GREATER_EQUAL, ">=", start_line, start_column);
            } else if (next == '>') {
                advance(lexer); // consume '>'
                advance(lexer); // consume '>'
                return token_create(TOKEN_RIGHT_SHIFT, ">>", start_line, start_column);
            }
            advance(lexer);
            return token_create(TOKEN_GREATER, ">", start_line, start_column);

        default: {
            // Single character operators and delimiters
            char lexeme[2] = {current, '\0'};
            TokenType type = TOKEN_UNKNOWN;

            switch (current) {
                case '*': type = TOKEN_MULTIPLY; break;
                case '/': type = TOKEN_DIVIDE; break;
                case '%': type = TOKEN_MODULO; break;
                case '(': type = TOKEN_LEFT_PAREN; break;
                case ')': type = TOKEN_RIGHT_PAREN; break;
                case '{': type = TOKEN_LEFT_BRACE; break;
                case '}': type = TOKEN_RIGHT_BRACE; break;
                case '[': type = TOKEN_LEFT_BRACKET; break;
                case ']': type = TOKEN_RIGHT_BRACKET; break;
                case ';': type = TOKEN_SEMICOLON; break;
                case ',': type = TOKEN_COMMA; break;
                case '.': type = TOKEN_DOT; break;
                case ':': type = TOKEN_COLON; break;
                case '?': type = TOKEN_QUESTION; break;
                case '\n': type = TOKEN_NEWLINE; break;
                default: type = TOKEN_UNKNOWN; break;
            }

            advance(lexer);
            return token_create(type, lexeme, start_line, start_column);
        }
    }
}