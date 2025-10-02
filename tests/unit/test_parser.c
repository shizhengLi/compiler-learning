#include "../../src/parser/parser.h"
#include "../test_framework.h"

TEST_SUITE(parser_creation) {
    // Test parser creation
    Lexer* lexer = lexer_create("int x = 42;");
    TEST_ASSERT_NOT_NULL(lexer, "Lexer should be created");

    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");
    TEST_ASSERT_NOT_NULL(parser->lexer, "Parser should have lexer");
    TEST_ASSERT(!parser->had_error, "Parser should not have errors initially");
    TEST_ASSERT_NULL(parser->last_error, "Parser should not have last error initially");

    parser_free(parser);
    lexer_free(lexer);

    // Test parser with NULL lexer
    parser = parser_create(NULL);
    TEST_ASSERT_NULL(parser, "Parser should not be created with NULL lexer");
}

TEST_SUITE(parser_basic_expressions) {
    // Test parsing integer literal
    Lexer* lexer = lexer_create("42");
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return AST node");
    TEST_ASSERT(node->type == NODE_LITERAL, "Should create literal node");
    TEST_ASSERT_EQ(42, node->data.literal.int_value, "Should parse integer value 42");
    TEST_ASSERT(!parser_had_error(parser), "Should not have parsing errors");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);

    // Test parsing identifier
    lexer = lexer_create("variable");
    parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return AST node");
    TEST_ASSERT(node->type == NODE_IDENTIFIER, "Should create identifier node");
    TEST_ASSERT_STR_EQ("variable", node->data.identifier_name, "Should parse identifier name");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

TEST_SUITE(parser_binary_expressions) {
    // Test parsing simple binary expression
    Lexer* lexer = lexer_create("1 + 2");
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return AST node");
    TEST_ASSERT(node->type == NODE_BINARY_EXPRESSION, "Should create binary expression node");
    TEST_ASSERT_NOT_NULL(node->data.binary.left, "Should have left operand");
    TEST_ASSERT_NOT_NULL(node->data.binary.right, "Should have right operand");
    TEST_ASSERT_STR_EQ("+", node->data.binary.operator, "Should have '+' operator");

    // Check left operand
    TEST_ASSERT(node->data.binary.left->type == NODE_LITERAL, "Left should be literal");
    TEST_ASSERT_EQ(1, node->data.binary.left->data.literal.int_value, "Left should be 1");

    // Check right operand
    TEST_ASSERT(node->data.binary.right->type == NODE_LITERAL, "Right should be literal");
    TEST_ASSERT_EQ(2, node->data.binary.right->data.literal.int_value, "Right should be 2");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

TEST_SUITE(parser_assignment) {
    // Test parsing assignment expression
    Lexer* lexer = lexer_create("x = 10");
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return AST node");
    TEST_ASSERT(node->type == NODE_ASSIGNMENT_EXPRESSION, "Should create assignment expression node");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

TEST_SUITE(parser_variable_declaration) {
    // Test parsing variable declaration
    Lexer* lexer = lexer_create("int x = 5;");
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return AST node");
    TEST_ASSERT(node->type == NODE_VARIABLE_DECLARATION, "Should create variable declaration node");
    TEST_ASSERT_STR_EQ("x", node->data.declaration.name, "Should parse variable name");
    TEST_ASSERT_STR_EQ("int", node->data.declaration.type_name, "Should parse type name");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

TEST_SUITE(parser_error_handling) {
    // Test parsing invalid syntax
    Lexer* lexer = lexer_create("int x = ;");
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return error node");
    TEST_ASSERT(node->type == NODE_ERROR, "Should create error node for invalid syntax");
    TEST_ASSERT(parser_had_error(parser), "Parser should have error flag set");
    TEST_ASSERT_NOT_NULL(parser_get_last_error(parser), "Parser should have last error");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

TEST_SUITE(parser_complex_expressions) {
    // Test parsing expression with precedence
    Lexer* lexer = lexer_create("1 + 2 * 3");
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return AST node");
    TEST_ASSERT(node->type == NODE_BINARY_EXPRESSION, "Should create binary expression node");
    TEST_ASSERT_STR_EQ("+", node->data.binary.operator, "Should have '+' as top operator");

    // Check right operand (should be 2 * 3)
    TEST_ASSERT(node->data.binary.right->type == NODE_BINARY_EXPRESSION, "Right should be binary expression");
    TEST_ASSERT_STR_EQ("*", node->data.binary.right->data.binary.operator, "Right should have '*' operator");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

TEST_SUITE(parser_node_utilities) {
    // Test AST node creation functions
    Token* token = token_create(TOKEN_INTEGER_LITERAL, "42", 1, 1);

    // Test binary node creation
    ASTNode* left = ast_node_create_literal_int(token, 1);
    ASTNode* right = ast_node_create_literal_int(token, 2);
    ASTNode* binary = ast_node_create_binary(token, left, right, "+");
    TEST_ASSERT_NOT_NULL(binary, "Binary node should be created");
    TEST_ASSERT(binary->type == NODE_BINARY_EXPRESSION, "Should be binary expression");
    TEST_ASSERT_STR_EQ("+", binary->data.binary.operator, "Should have '+' operator");

    // Test unary node creation
    ast_node_free(binary);
    ASTNode* operand = ast_node_create_literal_int(token, 5);
    ASTNode* unary = ast_node_create_unary(token, operand, "-");
    TEST_ASSERT_NOT_NULL(unary, "Unary node should be created");
    TEST_ASSERT(unary->type == NODE_UNARY_EXPRESSION, "Should be unary expression");
    TEST_ASSERT_STR_EQ("-", unary->data.unary.operator, "Should have '-' operator");

    // Test literal nodes
    ASTNode* int_literal = ast_node_create_literal_int(token, 42);
    TEST_ASSERT(int_literal->type == NODE_LITERAL, "Should be literal");
    TEST_ASSERT_EQ(42, int_literal->data.literal.int_value, "Should have correct int value");

    ASTNode* float_literal = ast_node_create_literal_float(token, 3.14f);
    TEST_ASSERT(float_literal->type == NODE_LITERAL, "Should be literal");
    TEST_ASSERT(3.14f == float_literal->data.literal.float_value, "Should have correct float value");

    ASTNode* string_literal = ast_node_create_literal_string(token, "hello");
    TEST_ASSERT(string_literal->type == NODE_LITERAL, "Should be literal");
    TEST_ASSERT_STR_EQ("hello", string_literal->data.literal.string_value, "Should have correct string value");

    ASTNode* identifier = ast_node_create_identifier(token, "x");
    TEST_ASSERT(identifier->type == NODE_IDENTIFIER, "Should be identifier");
    TEST_ASSERT_STR_EQ("x", identifier->data.identifier_name, "Should have correct identifier name");

    // Cleanup
    token_free(token);
    ast_node_free(left);
    ast_node_free(right);
    ast_node_free(operand);
    ast_node_free(unary);
    ast_node_free(int_literal);
    ast_node_free(float_literal);
    ast_node_free(string_literal);
    ast_node_free(identifier);
}

TEST_SUITE(parser_type_to_string) {
    TEST_ASSERT_STR_EQ("PROGRAM", node_type_to_string(NODE_PROGRAM), "PROGRAM should map to 'PROGRAM'");
    TEST_ASSERT_STR_EQ("LITERAL", node_type_to_string(NODE_LITERAL), "LITERAL should map to 'LITERAL'");
    TEST_ASSERT_STR_EQ("IDENTIFIER", node_type_to_string(NODE_IDENTIFIER), "IDENTIFIER should map to 'IDENTIFIER'");
    TEST_ASSERT_STR_EQ("BINARY_EXPRESSION", node_type_to_string(NODE_BINARY_EXPRESSION), "BINARY_EXPRESSION should map to 'BINARY_EXPRESSION'");
    TEST_ASSERT_STR_EQ("ERROR", node_type_to_string(NODE_ERROR), "ERROR should map to 'ERROR'");
}

// Add this test suite to the runner
void run_parser_basic_tests(void) {
    run_suite_parser_creation();
    run_suite_parser_basic_expressions();
    run_suite_parser_binary_expressions();
    run_suite_parser_assignment();
    run_suite_parser_variable_declaration();
    run_suite_parser_error_handling();
    run_suite_parser_complex_expressions();
    run_suite_parser_node_utilities();
    run_suite_parser_type_to_string();
}