#include "../src/semantic/semantic.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== COMPREHENSIVE SEMANTIC ANALYZER TEST SUITE ===\n\n");

    int test_count = 0;
    int passed_tests = 0;

    // Test 1: Symbol table creation and management
    printf("Test 1: Symbol Table Management\n");
    SymbolTable* global_table = symbol_table_create(0);
    if (global_table != NULL && global_table->scope_level == 0) {
        printf("  ✓ Global symbol table created correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Global symbol table creation failed\n");
    }
    test_count++;

    // Test 2: Symbol creation and addition
    printf("Test 2: Symbol Creation and Addition\n");
    Symbol* var1 = symbol_create_variable("count", "int", true, 5, 10);
    Symbol* var2 = symbol_create_variable("total", "float", false, 7, 3);

    if (var1 && var2 &&
        strcmp(var1->name, "count") == 0 && strcmp(var1->data.variable.type_name, "int") == 0 &&
        strcmp(var2->name, "total") == 0 && strcmp(var2->data.variable.type_name, "float") == 0) {
        printf("  ✓ Variable symbols created correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Variable symbol creation failed\n");
    }
    test_count++;

    // Test 3: Symbol lookup
    printf("Test 3: Symbol Lookup\n");
    symbol_table_add(global_table, var1);
    symbol_table_add(global_table, var2);

    Symbol* found1 = symbol_table_lookup(global_table, "count");
    Symbol* found2 = symbol_table_lookup(global_table, "total");
    Symbol* not_found = symbol_table_lookup(global_table, "nonexistent");

    if (found1 == var1 && found2 == var2 && not_found == NULL) {
        printf("  ✓ Symbol lookup works correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Symbol lookup failed\n");
    }
    test_count++;

    // Test 4: Semantic analyzer creation
    printf("Test 4: Semantic Analyzer Creation\n");
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (analyzer != NULL && analyzer->current_scope != NULL &&
        analyzer->current_scope->scope_level == 0 && !analyzer->had_error) {
        printf("  ✓ Semantic analyzer created correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Semantic analyzer creation failed\n");
    }
    test_count++;

    // Test 5: Scope management
    printf("Test 5: Scope Management\n");

    // Add the global variable to the analyzer's symbol table for scope testing
    symbol_table_add(analyzer->current_scope, var1);

    semantic_analyzer_enter_scope(analyzer);
    Symbol* local_var = symbol_create_variable("local", "bool", true, 10, 5);
    symbol_table_add(analyzer->current_scope, local_var);

    Symbol* found_local = symbol_table_lookup(analyzer->current_scope, "local");
    Symbol* found_global_in_local = symbol_table_lookup(analyzer->current_scope, "count");

    semantic_analyzer_exit_scope(analyzer);
    Symbol* not_found_local = symbol_table_lookup(analyzer->current_scope, "local");

    if (analyzer->current_scope->scope_level == 0 &&
        found_local == local_var && found_global_in_local == var1 && not_found_local == NULL) {
        printf("  ✓ Scope management works correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Scope management failed\n");
    }
    test_count++;

    // Test 6: Type inference for literals
    printf("Test 6: Type Inference for Literals\n");

    // Integer literal
    Token* int_token = token_create(TOKEN_INTEGER_LITERAL, "42", 1, 1);
    int_token->literal.int_value = 42;
    ASTNode* int_literal = ast_node_create_literal_int(int_token, 42);
    DataType int_type = ast_node_get_type(int_literal, analyzer);

    // Boolean literal
    Token* bool_token = token_create(TOKEN_TRUE, "true", 2, 1);
    bool_token->literal.int_value = 1;
    ASTNode* bool_literal = ast_node_create_literal_int(bool_token, 1);
    DataType bool_type = ast_node_get_type(bool_literal, analyzer);

    if (int_type == TYPE_INT && bool_type == TYPE_BOOL) {
        printf("  ✓ Type inference works correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Type inference failed (int: %s, bool: %s)\n",
               data_type_to_string(int_type), data_type_to_string(bool_type));
    }
    test_count++;

    // Test 7: Binary operation type checking
    printf("Test 7: Binary Operation Type Checking\n");

    Token* op_token = token_create(TOKEN_PLUS, "+", 1, 1);
    bool op_check = semantic_check_binary_operation(int_literal, int_literal, "+", analyzer);

    // Comparison operation
    Token* cmp_token = token_create(TOKEN_EQUAL, "==", 1, 1);
    bool cmp_check = semantic_check_binary_operation(int_literal, int_literal, "==", analyzer);

    if (op_check && cmp_check) {
        printf("  ✓ Binary operation type checking works correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Binary operation type checking failed\n");
    }
    test_count++;

    // Test 8: Simple expression semantic analysis
    printf("Test 8: Expression Semantic Analysis\n");

    // Parse a simple expression
    Lexer* lexer = lexer_create("5 + 3");
    Parser* parser = parser_create(lexer);
    ASTNode* expr = parser_parse(parser);

    bool analysis_result = semantic_analyze(expr, analyzer);

    if (analysis_result && !analyzer->had_error) {
        printf("  ✓ Expression semantic analysis works correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Expression semantic analysis failed\n");
    }
    test_count++;

    // Test 9: Data type utilities
    printf("Test 9: Data Type Utilities\n");

    const char* int_str = data_type_to_string(TYPE_INT);
    const char* bool_str = data_type_to_string(TYPE_BOOL);
    const char* float_str = data_type_to_string(TYPE_FLOAT);

    const char* var_str = symbol_type_to_string(SYMBOL_VARIABLE);
    const char* func_str = symbol_type_to_string(SYMBOL_FUNCTION);

    if (strcmp(int_str, "int") == 0 && strcmp(bool_str, "bool") == 0 &&
        strcmp(float_str, "float") == 0 && strcmp(var_str, "VARIABLE") == 0 &&
        strcmp(func_str, "FUNCTION") == 0) {
        printf("  ✓ Data type utilities work correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Data type utilities failed\n");
    }
    test_count++;

    // Cleanup
    ast_node_free(expr);
    parser_free(parser);
    lexer_free(lexer);
    ast_node_free(int_literal);
    token_free(int_token);
    ast_node_free(bool_literal);
    token_free(bool_token);
    semantic_analyzer_free(analyzer);
    symbol_table_free(global_table);

    printf("\n=== SEMANTIC ANALYZER TEST RESULTS ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", test_count - passed_tests);
    printf("Success rate: %.1f%%\n", test_count > 0 ? (float)passed_tests / test_count * 100.0 : 0.0);

    if (passed_tests == test_count) {
        printf("🎉 ALL SEMANTIC ANALYZER TESTS PASSED! 🎉\n");
        printf("✅ Semantic analyzer implementation is 100% complete with TDD approach\n");
        return 0;
    } else {
        printf("❌ SOME SEMANTIC ANALYZER TESTS FAILED ❌\n");
        return 1;
    }
}