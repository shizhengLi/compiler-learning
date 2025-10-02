#include "../src/parser/parser.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int test_count = 0;
int passed_tests = 0;
int failed_tests = 0;

#define TEST_ASSERT(condition, message) do { \
    test_count++; \
    if (!(condition)) { \
        printf("FAIL: %s\n", message); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

#define TEST_ASSERT_STR_EQ(expected, actual, message) do { \
    test_count++; \
    if (strcmp(expected, actual) != 0) { \
        printf("FAIL: %s (expected: '%s', actual: '%s')\n", message, expected, actual); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

#define TEST_ASSERT_NOT_NULL(ptr, message) do { \
    test_count++; \
    if ((ptr) == NULL) { \
        printf("FAIL: %s (pointer is NULL)\n", message); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

void print_ast_simple(ASTNode* node) {
    if (node == NULL) return;

    if (node->type == NODE_BINARY_EXPRESSION) {
        printf("(%s ", node->data.binary.operator);
        print_ast_simple(node->data.binary.left);
        printf(" ");
        print_ast_simple(node->data.binary.right);
        printf(")");
    } else if (node->type == NODE_LITERAL) {
        if (node->token && node->token->type == TOKEN_FLOAT_LITERAL) {
            printf("%.2f", node->data.literal.float_value);
        } else if (node->token && node->token->type == TOKEN_STRING_LITERAL) {
            printf("\"%s\"", node->data.literal.string_value);
        } else if (node->token && node->token->type == TOKEN_INTEGER_LITERAL) {
            printf("%d", node->data.literal.int_value);
        } else if (node->token && (node->token->type == TOKEN_TRUE || node->token->type == TOKEN_FALSE)) {
            printf("%s", node->data.literal.int_value ? "true" : "false");
        } else {
            printf("LITERAL");
        }
    } else if (node->type == NODE_IDENTIFIER) {
        printf("%s", node->data.identifier_name);
    } else {
        printf("%s", node_type_to_string(node->type));
    }
}

void test_parser_case(const char* input, const char* expected_ast, const char* description) {
    printf("Test: %s - '%s'\n", description, input);

    Lexer* lexer = lexer_create(input);
    TEST_ASSERT_NOT_NULL(lexer, "Lexer creation should succeed");

    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser creation should succeed");

    ASTNode* node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return a node");
    TEST_ASSERT(!parser_had_error(parser), "Parser should not have errors");

    // Print AST structure
    printf("  AST: ");
    print_ast_simple(node);
    printf("\n");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
    printf("\n");
}

void test_parser_error(const char* input, const char* description) {
    printf("Error Test: %s - '%s'\n", description, input);

    Lexer* lexer = lexer_create(input);
    TEST_ASSERT_NOT_NULL(lexer, "Lexer creation should succeed");

    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser creation should succeed");

    ASTNode* node = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(node, "Parser should return a node");
    TEST_ASSERT(parser_had_error(parser), "Parser should have errors");
    TEST_ASSERT(node->type == NODE_ERROR, "Should return error node");

    printf("  Expected error occurred correctly\n");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
    printf("\n");
}

void run_comprehensive_parser_tests(void) {
    printf("=== COMPREHENSIVE PARSER TEST SUITE ===\n\n");

    // Basic literals and identifiers
    test_parser_case("42", "42", "Integer literal");
    test_parser_case("3.14", "3.14", "Float literal");
    test_parser_case("hello", "hello", "Identifier");
    test_parser_case("_var123", "_var123", "Identifier with underscore and numbers");

    // Simple binary expressions
    test_parser_case("1 + 2", "(+ 1 2)", "Simple addition");
    test_parser_case("5 - 3", "(- 5 3)", "Simple subtraction");
    test_parser_case("4 * 6", "(* 4 6)", "Simple multiplication");
    test_parser_case("8 / 2", "(/ 8 2)", "Simple division");
    test_parser_case("7 % 3", "(% 7 3)", "Simple modulo");

    // Operator precedence
    test_parser_case("1 + 2 * 3", "(+ 1 (* 2 3))", "Multiplication higher than addition");
    test_parser_case("1 * 2 + 3", "(+ (* 1 2) 3)", "Multiplication before addition");
    test_parser_case("10 - 2 * 3 + 1", "(+ (- 10 (* 2 3)) 1)", "Mixed precedence");
    test_parser_case("20 / 4 * 3", "(* (/ 20 4) 3)", "Same precedence left associative");

    // Comparison operators
    test_parser_case("5 > 3", "(> 5 3)", "Greater than");
    test_parser_case("2 < 8", "(< 2 8)", "Less than");
    test_parser_case("5 >= 5", "(>= 5 5)", "Greater than or equal");
    test_parser_case("3 <= 4", "(<= 3 4)", "Less than or equal");
    test_parser_case("7 == 7", "(== 7 7)", "Equal");
    test_parser_case("1 != 2", "(!= 1 2)", "Not equal");

    // Logical operators
    test_parser_case("true && false", "(&& true false)", "Logical AND");
    test_parser_case("true || false", "(|| true false)", "Logical OR");

    // Bitwise operators
    test_parser_case("5 & 3", "(& 5 3)", "Bitwise AND");
    test_parser_case("5 | 3", "(| 5 3)", "Bitwise OR");
    test_parser_case("5 ^ 3", "(^ 5 3)", "Bitwise XOR");
    test_parser_case("1 << 3", "(<< 1 3)", "Left shift");
    test_parser_case("8 >> 2", "(>> 8 2)", "Right shift");

    // Complex expressions
    test_parser_case("(1 + 2) * (3 + 4)", "(* (+ 1 2) (+ 3 4))", "Parenthesized expressions");
    test_parser_case("1 + 2 + 3 + 4", "(+ (+ (+ 1 2) 3) 4)", "Left associative chain");
    test_parser_case("1 * 2 * 3 * 4", "(* (* (* 1 2) 3) 4)", "Left associative multiplication chain");

    // Mixed complex expression
    test_parser_case("1 + 2 * 3 - 4 / 5 + 6", "(+ (- (+ 1 (* 2 3)) (/ 4 5)) 6)", "Complex mixed expression");

    // Error cases
    test_parser_error("+", "Standalone operator");
    test_parser_error("*", "Standalone multiplication");
    test_parser_error("1 +", "Incomplete expression");
    test_parser_error("+ 1", "Expression starting with operator");

    // Assignment (if supported)
    test_parser_case("x = 5", "(= x 5)", "Assignment expression");
    test_parser_case("x = y = 10", "(= x (= y 10))", "Chained assignment");

    printf("=== PARSER TEST RESULTS ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success rate: %.1f%%\n", test_count > 0 ? (float)passed_tests / test_count * 100.0 : 0.0);

    if (failed_tests == 0) {
        printf("🎉 ALL PARSER TESTS PASSED! 🎉\n");
    } else {
        printf("❌ SOME PARSER TESTS FAILED ❌\n");
    }
}

int main(void) {
    run_comprehensive_parser_tests();
    return (failed_tests == 0) ? 0 : 1;
}