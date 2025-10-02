#include "../../src/lexer/token.h"
#include "../test_framework.h"

TEST_SUITE(token_creation) {
    // Test basic token creation
    Token* token = token_create(TOKEN_PLUS, "+", 1, 1);
    TEST_ASSERT_NOT_NULL(token, "Token should be created");
    TEST_ASSERT(token->type == TOKEN_PLUS, "Token type should be PLUS");
    TEST_ASSERT_STR_EQ("+", token->lexeme, "Token lexeme should be '+'");
    TEST_ASSERT_EQ(1, token->line, "Token line should be 1");
    TEST_ASSERT_EQ(1, token->column, "Token column should be 1");
    token_free(token);

    // Test identifier token
    token = token_create(TOKEN_IDENTIFIER, "variable", 2, 5);
    TEST_ASSERT_NOT_NULL(token, "Identifier token should be created");
    TEST_ASSERT(token->type == TOKEN_IDENTIFIER, "Token type should be IDENTIFIER");
    TEST_ASSERT_STR_EQ("variable", token->lexeme, "Token lexeme should be 'variable'");
    TEST_ASSERT_EQ(2, token->line, "Token line should be 2");
    TEST_ASSERT_EQ(5, token->column, "Token column should be 5");
    token_free(token);

    // Test EOF token
    token = token_create(TOKEN_EOF, "", 10, 20);
    TEST_ASSERT_NOT_NULL(token, "EOF token should be created");
    TEST_ASSERT(token->type == TOKEN_EOF, "Token type should be EOF");
    TEST_ASSERT_STR_EQ("", token->lexeme, "EOF token lexeme should be empty");
    TEST_ASSERT_EQ(10, token->line, "EOF token line should be 10");
    TEST_ASSERT_EQ(20, token->column, "EOF token column should be 20");
    token_free(token);
}

TEST_SUITE(token_literals) {
    // Test integer literal token
    Token* token = token_create_with_literal(TOKEN_INTEGER_LITERAL, "42", 1, 1);
    TEST_ASSERT_NOT_NULL(token, "Integer literal token should be created");
    TEST_ASSERT(token->type == TOKEN_INTEGER_LITERAL, "Token type should be INTEGER_LITERAL");
    TEST_ASSERT_STR_EQ("42", token->lexeme, "Token lexeme should be '42'");
    TEST_ASSERT_EQ(42, token->literal.int_value, "Integer literal value should be 42");
    token_free(token);

    // Test float literal token
    token = token_create_with_literal(TOKEN_FLOAT_LITERAL, "3.14", 1, 1);
    TEST_ASSERT_NOT_NULL(token, "Float literal token should be created");
    TEST_ASSERT(token->type == TOKEN_FLOAT_LITERAL, "Token type should be FLOAT_LITERAL");
    TEST_ASSERT_STR_EQ("3.14", token->lexeme, "Token lexeme should be '3.14'");
    TEST_ASSERT(3.14f == token->literal.float_value, "Float literal value should be 3.14");
    token_free(token);

    // Test string literal token
    token = token_create_with_literal(TOKEN_STRING_LITERAL, "\"hello\"", 1, 1);
    TEST_ASSERT_NOT_NULL(token, "String literal token should be created");
    TEST_ASSERT(token->type == TOKEN_STRING_LITERAL, "Token type should be STRING_LITERAL");
    TEST_ASSERT_STR_EQ("\"hello\"", token->lexeme, "Token lexeme should be '\"hello\"'");
    TEST_ASSERT_STR_EQ("hello", token->literal.string_value, "String literal value should be 'hello'");
    token_free(token);

    // Test char literal token
    token = token_create_with_literal(TOKEN_CHAR_LITERAL, "'a'", 1, 1);
    TEST_ASSERT_NOT_NULL(token, "Char literal token should be created");
    TEST_ASSERT(token->type == TOKEN_CHAR_LITERAL, "Token type should be CHAR_LITERAL");
    TEST_ASSERT_STR_EQ("'a'", token->lexeme, "Token lexeme should be \"'a'\"");
    TEST_ASSERT_EQ('a', token->literal.char_value, "Char literal value should be 'a'");
    token_free(token);
}

TEST_SUITE(token_keywords) {
    // Test keyword recognition
    TokenType type;
    TEST_ASSERT(token_is_keyword("int", &type), "'int' should be recognized as keyword");
    TEST_ASSERT(type == TOKEN_INT, "'int' should map to TOKEN_INT");

    TEST_ASSERT(token_is_keyword("if", &type), "'if' should be recognized as keyword");
    TEST_ASSERT(type == TOKEN_IF, "'if' should map to TOKEN_IF");

    TEST_ASSERT(token_is_keyword("while", &type), "'while' should be recognized as keyword");
    TEST_ASSERT(type == TOKEN_WHILE, "'while' should map to TOKEN_WHILE");

    TEST_ASSERT(token_is_keyword("return", &type), "'return' should be recognized as keyword");
    TEST_ASSERT(type == TOKEN_RETURN, "'return' should map to TOKEN_RETURN");

    TEST_ASSERT(token_is_keyword("true", &type), "'true' should be recognized as keyword");
    TEST_ASSERT(type == TOKEN_TRUE, "'true' should map to TOKEN_TRUE");

    // Test non-keyword
    TEST_ASSERT(!token_is_keyword("variable", &type), "'variable' should not be recognized as keyword");
    TEST_ASSERT(!token_is_keyword("ifx", &type), "'ifx' should not be recognized as keyword");
    TEST_ASSERT(!token_is_keyword("", &type), "Empty string should not be recognized as keyword");
    TEST_ASSERT(!token_is_keyword(NULL, &type), "NULL should not be recognized as keyword");
}

TEST_SUITE(token_type_to_string) {
    TEST_ASSERT_STR_EQ("EOF", token_type_to_string(TOKEN_EOF), "EOF token should map to 'EOF'");
    TEST_ASSERT_STR_EQ("PLUS", token_type_to_string(TOKEN_PLUS), "PLUS token should map to 'PLUS'");
    TEST_ASSERT_STR_EQ("IDENTIFIER", token_type_to_string(TOKEN_IDENTIFIER), "IDENTIFIER token should map to 'IDENTIFIER'");
    TEST_ASSERT_STR_EQ("INTEGER_LITERAL", token_type_to_string(TOKEN_INTEGER_LITERAL), "INTEGER_LITERAL token should map to 'INTEGER_LITERAL'");
    TEST_ASSERT_STR_EQ("UNKNOWN", token_type_to_string(TOKEN_UNKNOWN), "UNKNOWN token should map to 'UNKNOWN'");
}

TEST_SUITE(token_edge_cases) {
    // Test NULL lexeme
    Token* token = token_create(TOKEN_PLUS, NULL, 1, 1);
    TEST_ASSERT_NOT_NULL(token, "Token with NULL lexeme should be created");
    TEST_ASSERT_NULL(token->lexeme, "Token lexeme should be NULL");
    token_free(token);

    // Test large line and column numbers
    token = token_create(TOKEN_IDENTIFIER, "x", 999999, 888888);
    TEST_ASSERT_NOT_NULL(token, "Token with large line/column should be created");
    TEST_ASSERT_EQ(999999, token->line, "Token line should be 999999");
    TEST_ASSERT_EQ(888888, token->column, "Token column should be 888888");
    token_free(token);
}

// Add this test suite to the runner
void run_token_tests(void) {
    run_suite_token_creation();
    run_suite_token_literals();
    run_suite_token_keywords();
    run_suite_token_type_to_string();
    run_suite_token_edge_cases();
}