#include "../src/parser/parser.h"
#include "test_framework_simple.h"

TEST_SUITE(parser_basic_creation) {
    // Test parser creation
    Lexer* lexer = lexer_create("42");
    TEST_ASSERT_NOT_NULL(lexer, "Lexer should be created");

    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");
    TEST_ASSERT_NOT_NULL(parser->lexer, "Parser should have lexer");
    TEST_ASSERT(!parser->had_error, "Parser should not have errors initially");

    parser_free(parser);
    lexer_free(lexer);
}

TEST_SUITE(parser_basic_parsing) {
    // Test parsing integer literal
    Lexer* lexer = lexer_create("42");
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return AST node");
    TEST_ASSERT(node->type == NODE_LITERAL, "Should create literal node");
    TEST_ASSERT_EQ(42, node->data.literal.int_value, "Should parse integer value 42");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

TEST_SUITE(parser_node_creation) {
    // Test AST node creation functions
    Token* token = token_create(TOKEN_INTEGER_LITERAL, "42", 1, 1);

    // Test literal node creation
    ASTNode* int_literal = ast_node_create_literal_int(token, 42);
    TEST_ASSERT_NOT_NULL(int_literal, "Literal node should be created");
    TEST_ASSERT(int_literal->type == NODE_LITERAL, "Should be literal");
    TEST_ASSERT_EQ(42, int_literal->data.literal.int_value, "Should have correct int value");

    // Test identifier node creation
    ASTNode* identifier = ast_node_create_identifier(token, "x");
    TEST_ASSERT_NOT_NULL(identifier, "Identifier node should be created");
    TEST_ASSERT(identifier->type == NODE_IDENTIFIER, "Should be identifier");
    TEST_ASSERT_STR_EQ("x", identifier->data.identifier_name, "Should have correct identifier name");

    // Cleanup
    token_free(token);
    ast_node_free(int_literal);
    ast_node_free(identifier);
}

// Add this test suite to the runner
void run_parser_basic_tests(void) {
    run_suite_parser_basic_creation();
    run_suite_parser_basic_parsing();
    run_suite_parser_node_creation();
}

int main(void) {
    reset_test_counters();

    printf("=== BASIC PARSER TEST SUITE ===\n\n");

    run_parser_basic_tests();

    print_test_results();
    return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}