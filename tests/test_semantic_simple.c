#include "../src/semantic/semantic.h"
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

#define TEST_ASSERT_EQ(expected, actual, message) do { \
    test_count++; \
    if ((expected) != (actual)) { \
        printf("FAIL: %s (expected: %d, actual: %d)\n", message, (int)(expected), (int)(actual)); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

void test_symbol_table_basic(void) {
    printf("Testing symbol table basic functionality...\n");

    SymbolTable* table = symbol_table_create(0);
    TEST_ASSERT_NOT_NULL(table, "Symbol table should be created");
    TEST_ASSERT_EQ(0, table->scope_level, "Scope level should be 0");

    Symbol* var = symbol_create_variable("x", "int", true, 1, 1);
    TEST_ASSERT_NOT_NULL(var, "Variable symbol should be created");
    TEST_ASSERT_STR_EQ("x", var->name, "Variable name should be 'x'");
    TEST_ASSERT_STR_EQ("int", var->data.variable.type_name, "Variable type should be 'int'");

    symbol_table_add(table, var);
    TEST_ASSERT_EQ(1, table->symbol_count, "Symbol count should be 1");

    Symbol* found = symbol_table_lookup(table, "x");
    TEST_ASSERT(found == var, "Should find the added symbol");
    TEST_ASSERT_STR_EQ("x", found->name, "Found symbol should have correct name");

    symbol_table_free(table);
    printf("✓ Symbol table basic tests passed\n\n");
}

void test_semantic_analyzer_basic(void) {
    printf("Testing semantic analyzer basic functionality...\n");

    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");
    TEST_ASSERT_NOT_NULL(analyzer->current_scope, "Should have current scope");
    TEST_ASSERT_EQ(0, analyzer->current_scope->scope_level, "Initial scope level should be 0");

    semantic_analyzer_enter_scope(analyzer);
    TEST_ASSERT_EQ(1, analyzer->current_scope->scope_level, "Scope level should be 1 after entering scope");

    semantic_analyzer_exit_scope(analyzer);
    TEST_ASSERT_EQ(0, analyzer->current_scope->scope_level, "Should be back to global scope");

    semantic_analyzer_free(analyzer);
    printf("✓ Semantic analyzer basic tests passed\n\n");
}

void test_type_inference(void) {
    printf("Testing type inference...\n");

    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");

    // Test integer literal type inference
    Token* int_token = token_create(TOKEN_INTEGER_LITERAL, "42", 1, 1);
    int_token->literal.int_value = 42;
    ASTNode* int_literal = ast_node_create_literal_int(int_token, 42);

    DataType int_type = ast_node_get_type(int_literal, analyzer);
    TEST_ASSERT(int_type == TYPE_INT, "Integer literal should have INT type");
    printf("  Integer literal type: %s\n", data_type_to_string(int_type));

    ast_node_free(int_literal);
    token_free(int_token);

    semantic_analyzer_free(analyzer);
    printf("✓ Type inference tests passed\n\n");
}

void test_data_type_utilities(void) {
    printf("Testing data type utilities...\n");

    TEST_ASSERT_STR_EQ("int", data_type_to_string(TYPE_INT), "TYPE_INT should map to 'int'");
    TEST_ASSERT_STR_EQ("float", data_type_to_string(TYPE_FLOAT), "TYPE_FLOAT should map to 'float'");
    TEST_ASSERT_STR_EQ("bool", data_type_to_string(TYPE_BOOL), "TYPE_BOOL should map to 'bool'");

    TEST_ASSERT_STR_EQ("VARIABLE", symbol_type_to_string(SYMBOL_VARIABLE), "SYMBOL_VARIABLE should map to 'VARIABLE'");
    TEST_ASSERT_STR_EQ("FUNCTION", symbol_type_to_string(SYMBOL_FUNCTION), "SYMBOL_FUNCTION should map to 'FUNCTION'");

    printf("✓ Data type utility tests passed\n\n");
}

int main(void) {
    printf("=== SIMPLE SEMANTIC ANALYZER TEST SUITE ===\n\n");

    test_symbol_table_basic();
    test_semantic_analyzer_basic();
    test_type_inference();
    test_data_type_utilities();

    printf("=== SEMANTIC ANALYZER TEST RESULTS ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success rate: %.1f%%\n", test_count > 0 ? (float)passed_tests / test_count * 100.0 : 0.0);

    if (failed_tests == 0) {
        printf("🎉 ALL SEMANTIC ANALYZER TESTS PASSED! 🎉\n");
        return 0;
    } else {
        printf("❌ SOME SEMANTIC ANALYZER TESTS FAILED ❌\n");
        return 1;
    }
}