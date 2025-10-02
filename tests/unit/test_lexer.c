#include "../../src/lexer/lexer.h"
#include "../test_framework.h"

TEST_SUITE(lexer_creation) {
    // Test basic lexer creation
    Lexer* lexer = lexer_create("int x = 42;");
    TEST_ASSERT_NOT_NULL(lexer, "Lexer should be created");
    TEST_ASSERT_STR_EQ("int x = 42;", lexer->source, "Source should be set correctly");
    TEST_ASSERT_EQ(0, lexer->position, "Position should start at 0");
    TEST_ASSERT_EQ(1, lexer->line, "Line should start at 1");
    TEST_ASSERT_EQ(1, lexer->column, "Column should start at 1");
    TEST_ASSERT_EQ('i', lexer->current_char, "Current character should be 'i'");
    TEST_ASSERT(!lexer->had_error, "Error flag should be false initially");
    TEST_ASSERT_NULL(lexer->last_error, "Last error should be NULL initially");
    lexer_free(lexer);

    // Test empty source
    lexer = lexer_create("");
    TEST_ASSERT_NOT_NULL(lexer, "Lexer with empty source should be created");
    TEST_ASSERT_STR_EQ("", lexer->source, "Source should be empty");
    TEST_ASSERT_EQ('\0', lexer->current_char, "Current character should be null terminator");
    lexer_free(lexer);

    // Test NULL source
    lexer = lexer_create(NULL);
    TEST_ASSERT_NOT_NULL(lexer, "Lexer with NULL source should be created");
    TEST_ASSERT_NULL(lexer->source, "Source should be NULL");
    lexer_free(lexer);
}

TEST_SUITE(lexer_basic_tokens) {
    // Test simple identifier
    Lexer* lexer = lexer_create("variable");
    Token* token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Token should be generated");
    TEST_ASSERT(token->type == TOKEN_IDENTIFIER, "Should recognize identifier");
    TEST_ASSERT_STR_EQ("variable", token->lexeme, "Lexeme should be 'variable'");
    TEST_ASSERT_EQ(1, token->line, "Token line should be 1");
    TEST_ASSERT_EQ(1, token->column, "Token column should be 1");
    token_free(token);

    // Test EOF
    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "EOF token should be generated");
    TEST_ASSERT(token->type == TOKEN_EOF, "Should recognize EOF");
    token_free(token);

    lexer_free(lexer);
}

TEST_SUITE(lexer_keywords) {
    // Test all keywords
    const char* keywords[] = {
        "int", "float", "char", "bool", "void",
        "if", "else", "while", "for", "return",
        "break", "continue", "true", "false", "null"
    };

    TokenType expected_types[] = {
        TOKEN_INT, TOKEN_FLOAT, TOKEN_CHAR, TOKEN_BOOL, TOKEN_VOID,
        TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE, TOKEN_FOR, TOKEN_RETURN,
        TOKEN_BREAK, TOKEN_CONTINUE, TOKEN_TRUE, TOKEN_FALSE, TOKEN_NULL
    };

    int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < keyword_count; i++) {
        char source[32];
        sprintf(source, "%s", keywords[i]);

        Lexer* lexer = lexer_create(source);
        Token* token = lexer_next_token(lexer);

        char test_msg[128];
        sprintf(test_msg, "Should recognize keyword '%s'", keywords[i]);
        TEST_ASSERT_NOT_NULL(token, test_msg);
        TEST_ASSERT(token->type == expected_types[i], test_msg);
        TEST_ASSERT_STR_EQ(keywords[i], token->lexeme, test_msg);

        token_free(token);
        lexer_free(lexer);
    }
}

TEST_SUITE(lexer_literals) {
    // Test integer literals
    Lexer* lexer = lexer_create("42");
    Token* token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Integer token should be generated");
    TEST_ASSERT(token->type == TOKEN_INTEGER_LITERAL, "Should recognize integer literal");
    TEST_ASSERT_STR_EQ("42", token->lexeme, "Lexeme should be '42'");
    TEST_ASSERT_EQ(42, token->literal.int_value, "Integer value should be 42");
    token_free(token);
    lexer_free(lexer);

    // Test float literals
    lexer = lexer_create("3.14");
    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Float token should be generated");
    TEST_ASSERT(token->type == TOKEN_FLOAT_LITERAL, "Should recognize float literal");
    TEST_ASSERT_STR_EQ("3.14", token->lexeme, "Lexeme should be '3.14'");
    TEST_ASSERT(3.14f == token->literal.float_value, "Float value should be 3.14");
    token_free(token);
    lexer_free(lexer);

    // Test string literals
    lexer = lexer_create("\"hello world\"");
    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "String token should be generated");
    TEST_ASSERT(token->type == TOKEN_STRING_LITERAL, "Should recognize string literal");
    TEST_ASSERT_STR_EQ("\"hello world\"", token->lexeme, "Lexeme should be '\"hello world\"'");
    TEST_ASSERT_STR_EQ("hello world", token->literal.string_value, "String value should be 'hello world'");
    token_free(token);
    lexer_free(lexer);

    // Test char literals
    lexer = lexer_create("'a'");
    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Char token should be generated");
    TEST_ASSERT(token->type == TOKEN_CHAR_LITERAL, "Should recognize char literal");
    TEST_ASSERT_STR_EQ("'a'", token->lexeme, "Lexeme should be \"'a'\"");
    TEST_ASSERT_EQ('a', token->literal.char_value, "Char value should be 'a'");
    token_free(token);
    lexer_free(lexer);
}

TEST_SUITE(lexer_operators) {
    // Test single character operators
    const char* single_ops[] = {
        "+", "-", "*", "/", "%", "=", "<", ">", "!",
        "&", "|", "^", "~", "(", ")", "{", "}", "[", "]",
        ";", ",", ".", ":", "?"
    };

    TokenType expected_types[] = {
        TOKEN_PLUS, TOKEN_MINUS, TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_MODULO, TOKEN_ASSIGN,
        TOKEN_LESS, TOKEN_GREATER, TOKEN_LOGICAL_NOT, TOKEN_BITWISE_AND, TOKEN_BITWISE_OR,
        TOKEN_BITWISE_XOR, TOKEN_BITWISE_NOT, TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
        TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
        TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_DOT, TOKEN_COLON, TOKEN_QUESTION
    };

    int op_count = sizeof(single_ops) / sizeof(single_ops[0]);

    for (int i = 0; i < op_count; i++) {
        Lexer* lexer = lexer_create(single_ops[i]);
        Token* token = lexer_next_token(lexer);

        char test_msg[128];
        sprintf(test_msg, "Should recognize operator '%s'", single_ops[i]);
        TEST_ASSERT_NOT_NULL(token, test_msg);
        TEST_ASSERT(token->type == expected_types[i], test_msg);
        TEST_ASSERT_STR_EQ(single_ops[i], token->lexeme, test_msg);

        token_free(token);
        lexer_free(lexer);
    }

    // Test multi-character operators
    const char* multi_ops[][2] = {
        {"==", "EQUAL"},
        {"!=", "NOT_EQUAL"},
        {"<=", "LESS_EQUAL"},
        {">=", "GREATER_EQUAL"},
        {"&&", "LOGICAL_AND"},
        {"||", "LOGICAL_OR"},
        {"++", "INCREMENT"},
        {"--", "DECREMENT"},
        {"<<", "LEFT_SHIFT"},
        {">>", "RIGHT_SHIFT"}
    };

    TokenType multi_types[] = {
        TOKEN_EQUAL, TOKEN_NOT_EQUAL, TOKEN_LESS_EQUAL, TOKEN_GREATER_EQUAL,
        TOKEN_LOGICAL_AND, TOKEN_LOGICAL_OR, TOKEN_INCREMENT, TOKEN_DECREMENT,
        TOKEN_LEFT_SHIFT, TOKEN_RIGHT_SHIFT
    };

    int multi_op_count = sizeof(multi_ops) / sizeof(multi_ops[0]);

    for (int i = 0; i < multi_op_count; i++) {
        Lexer* lexer = lexer_create(multi_ops[i][0]);
        Token* token = lexer_next_token(lexer);

        char test_msg[128];
        sprintf(test_msg, "Should recognize operator '%s'", multi_ops[i][0]);
        TEST_ASSERT_NOT_NULL(token, test_msg);
        TEST_ASSERT(token->type == multi_types[i], test_msg);
        TEST_ASSERT_STR_EQ(multi_ops[i][0], token->lexeme, test_msg);

        token_free(token);
        lexer_free(lexer);
    }
}

TEST_SUITE(lexer_complex_input) {
    // Test a simple expression
    Lexer* lexer = lexer_create("int result = a + b * 42;");

    Token* token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get first token");
    TEST_ASSERT(token->type == TOKEN_INT, "First token should be INT");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get second token");
    TEST_ASSERT(token->type == TOKEN_IDENTIFIER, "Second token should be IDENTIFIER");
    TEST_ASSERT_STR_EQ("result", token->lexeme, "Identifier should be 'result'");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get third token");
    TEST_ASSERT(token->type == TOKEN_ASSIGN, "Third token should be ASSIGN");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get fourth token");
    TEST_ASSERT(token->type == TOKEN_IDENTIFIER, "Fourth token should be IDENTIFIER");
    TEST_ASSERT_STR_EQ("a", token->lexeme, "Identifier should be 'a'");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get fifth token");
    TEST_ASSERT(token->type == TOKEN_PLUS, "Fifth token should be PLUS");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get sixth token");
    TEST_ASSERT(token->type == TOKEN_IDENTIFIER, "Sixth token should be IDENTIFIER");
    TEST_ASSERT_STR_EQ("b", token->lexeme, "Identifier should be 'b'");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get seventh token");
    TEST_ASSERT(token->type == TOKEN_MULTIPLY, "Seventh token should be MULTIPLY");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get eighth token");
    TEST_ASSERT(token->type == TOKEN_INTEGER_LITERAL, "Eighth token should be INTEGER_LITERAL");
    TEST_ASSERT_EQ(42, token->literal.int_value, "Integer value should be 42");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get ninth token");
    TEST_ASSERT(token->type == TOKEN_SEMICOLON, "Ninth token should be SEMICOLON");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get EOF token");
    TEST_ASSERT(token->type == TOKEN_EOF, "Last token should be EOF");
    token_free(token);

    lexer_free(lexer);
}

TEST_SUITE(lexer_line_column_tracking) {
    // Test multi-line input
    Lexer* lexer = lexer_create("line1\nline2\nline3");

    Token* token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get first token");
    TEST_ASSERT(token->type == TOKEN_IDENTIFIER, "Should recognize identifier");
    TEST_ASSERT_EQ(1, token->line, "Token should be on line 1");
    TEST_ASSERT_EQ(1, token->column, "Token should start at column 1");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get newline token");
    TEST_ASSERT(token->type == TOKEN_NEWLINE, "Should recognize newline");
    token_free(token);

    token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get second line token");
    TEST_ASSERT(token->type == TOKEN_IDENTIFIER, "Should recognize identifier");
    TEST_ASSERT_EQ(2, token->line, "Token should be on line 2");
    TEST_ASSERT_EQ(1, token->column, "Token should start at column 1");
    token_free(token);

    lexer_free(lexer);
}

TEST_SUITE(lexer_error_handling) {
    // Test unterminated string
    Lexer* lexer = lexer_create("\"unterminated string");
    Token* token = lexer_next_token(lexer);
    TEST_ASSERT_NOT_NULL(token, "Should get a token even with error");
    TEST_ASSERT(lexer->had_error, "Lexer should have error");
    TEST_ASSERT_NOT_NULL(lexer->last_error, "Should have error message");
    token_free(token);
    lexer_free(lexer);
}

// Update the lexer test runner
void run_lexer_tests(void) {
    run_suite_lexer_creation();
    run_suite_lexer_basic_tokens();
    run_suite_lexer_keywords();
    run_suite_lexer_literals();
    run_suite_lexer_operators();
    run_suite_lexer_complex_input();
    run_suite_lexer_line_column_tracking();
    run_suite_lexer_error_handling();
}