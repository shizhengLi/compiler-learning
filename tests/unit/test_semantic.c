#include "../../src/semantic/semantic.h"
#include "../test_framework.h"

TEST_SUITE(symbol_table_creation) {
    // Test symbol table creation
    SymbolTable* table = symbol_table_create(0);
    TEST_ASSERT_NOT_NULL(table, "Symbol table should be created");
    TEST_ASSERT_EQ(0, table->scope_level, "Scope level should be 0");
    TEST_ASSERT_NULL(table->parent, "Parent should be NULL initially");
    TEST_ASSERT_EQ(0, table->symbol_count, "Symbol count should be 0");
    TEST_ASSERT(table->capacity > 0, "Capacity should be positive");

    symbol_table_free(table);
}

TEST_SUITE(symbol_creation) {
    // Test variable symbol creation
    Symbol* var = symbol_create_variable("x", "int", true, 1, 1);
    TEST_ASSERT_NOT_NULL(var, "Variable symbol should be created");
    TEST_ASSERT_STR_EQ("x", var->name, "Symbol name should be 'x'");
    TEST_ASSERT_EQ(SYMBOL_VARIABLE, var->type, "Symbol type should be VARIABLE");
    TEST_ASSERT_STR_EQ("int", var->data.variable.type_name, "Variable type should be 'int'");
    TEST_ASSERT(var->data.variable.is_mutable, "Variable should be mutable");
    TEST_ASSERT_EQ(1, var->line, "Line should be 1");
    TEST_ASSERT_EQ(1, var->column, "Column should be 1");

    symbol_free(var);
}

TEST_SUITE(symbol_table_add_lookup) {
    // Test adding and looking up symbols
    SymbolTable* table = symbol_table_create(0);
    TEST_ASSERT_NOT_NULL(table, "Symbol table should be created");

    // Add a variable symbol
    Symbol* var = symbol_create_variable("my_var", "float", false, 5, 10);
    TEST_ASSERT_NOT_NULL(var, "Variable symbol should be created");

    Symbol* added = symbol_table_add(table, var);
    TEST_ASSERT(var == added, "Should return the added symbol");
    TEST_ASSERT_EQ(1, table->symbol_count, "Symbol count should be 1");

    // Look up the symbol
    Symbol* found = symbol_table_lookup(table, "my_var");
    TEST_ASSERT(var == found, "Should find the added symbol");
    TEST_ASSERT_STR_EQ("my_var", found->name, "Found symbol should have correct name");
    TEST_ASSERT_STR_EQ("float", found->data.variable.type_name, "Found symbol should have correct type");

    // Test lookup of non-existent symbol
    Symbol* not_found = symbol_table_lookup(table, "non_existent");
    TEST_ASSERT_NULL(not_found, "Should not find non-existent symbol");

    symbol_table_free(table);
}

TEST_SUITE(semantic_analyzer_creation) {
    // Test semantic analyzer creation
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");
    TEST_ASSERT_NOT_NULL(analyzer->current_scope, "Should have current scope");
    TEST_ASSERT_EQ(0, analyzer->current_scope->scope_level, "Initial scope level should be 0");
    TEST_ASSERT(!analyzer->had_error, "Should not have errors initially");
    TEST_ASSERT_NULL(analyzer->last_error, "Should not have last error initially");

    semantic_analyzer_free(analyzer);
}

TEST_SUITE(scope_management) {
    // Test entering and exiting scopes
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");

    // Add a symbol to global scope
    Symbol* global_var = symbol_create_variable("global", "int", true, 1, 1);
    symbol_table_add(analyzer->current_scope, global_var);

    // Enter new scope
    semantic_analyzer_enter_scope(analyzer);
    TEST_ASSERT_EQ(1, analyzer->current_scope->scope_level, "Scope level should be 1");
    TEST_ASSERT_NOT_NULL(analyzer->current_scope->parent, "Should have parent scope");

    // Add a symbol to local scope
    Symbol* local_var = symbol_create_variable("local", "float", false, 5, 1);
    symbol_table_add(analyzer->current_scope, local_var);

    // Look up local symbol
    Symbol* found_local = symbol_table_lookup(analyzer->current_scope, "local");
    TEST_ASSERT_NOT_NULL(found_local, "Should find local symbol");

    // Look up global symbol from local scope
    Symbol* found_global = symbol_table_lookup(analyzer->current_scope, "global");
    TEST_ASSERT_NOT_NULL(found_global, "Should find global symbol from local scope");

    // Exit scope
    semantic_analyzer_exit_scope(analyzer);
    TEST_ASSERT_EQ(0, analyzer->current_scope->scope_level, "Should be back to global scope");

    // Local symbol should not be found anymore
    Symbol* not_found_local = symbol_table_lookup(analyzer->current_scope, "local");
    TEST_ASSERT_NULL(not_found_local, "Should not find local symbol after exiting scope");

    semantic_analyzer_free(analyzer);
}

TEST_SUITE(type_inference) {
    // Test type inference for literals
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");

    // Test integer literal
    Token* int_token = token_create(TOKEN_INTEGER_LITERAL, "42", 1, 1);
    int_token->literal.int_value = 42;
    ASTNode* int_literal = ast_node_create_literal_int(int_token, 42);

    DataType int_type = ast_node_get_type(int_literal, analyzer);
    TEST_ASSERT_EQ(TYPE_INT, int_type, "Integer literal should have INT type");

    ast_node_free(int_literal);
    token_free(int_token);

    // Test boolean literal (true)
    Token* true_token = token_create(TOKEN_TRUE, "true", 1, 1);
    true_token->literal.int_value = 1;
    ASTNode* true_literal = ast_node_create_literal_int(true_token, 1);

    DataType bool_type = ast_node_get_type(true_literal, analyzer);
    TEST_ASSERT_EQ(TYPE_BOOL, bool_type, "Boolean literal should have BOOL type");

    ast_node_free(true_literal);
    token_free(true_token);

    semantic_analyzer_free(analyzer);
}

TEST_SUITE(binary_operation_type_checking) {
    // Test binary operation type checking
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");

    // Create two integer literals
    Token* token1 = token_create(TOKEN_INTEGER_LITERAL, "5", 1, 1);
    Token* token2 = token_create(TOKEN_INTEGER_LITERAL, "3", 1, 1);
    token1->literal.int_value = 5;
    token2->literal.int_value = 3;

    ASTNode* left = ast_node_create_literal_int(token1, 5);
    ASTNode* right = ast_node_create_literal_int(token2, 3);
    ASTNode* binary = ast_node_create_binary(token1, left, right, "+");

    // Check binary operation
    bool result = semantic_check_binary_operation(left, right, "+", analyzer);
    TEST_ASSERT(result, "Integer + Integer should be valid");

    ast_node_free(binary);
    semantic_analyzer_free(analyzer);
}

TEST_SUITE(semantic_analysis_simple) {
    // Test semantic analysis of simple expressions
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");

    // Create a simple expression: 1 + 2
    Lexer* lexer = lexer_create("1 + 2");
    Parser* parser = parser_create(lexer);
    ASTNode* ast = parser_parse(parser);

    TEST_ASSERT_NOT_NULL(ast, "Parser should return AST");
    TEST_ASSERT_EQ(NODE_BINARY_EXPRESSION, ast->type, "Should be binary expression");

    // Analyze the AST
    bool analysis_result = semantic_analyze(ast, analyzer);
    TEST_ASSERT(analysis_result, "Semantic analysis should succeed");
    TEST_ASSERT(!analyzer->had_error, "Should not have semantic errors");

    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    semantic_analyzer_free(analyzer);
}

TEST_SUITE(data_type_utility) {
    // Test data type utility functions
    TEST_ASSERT_STR_EQ("int", data_type_to_string(TYPE_INT), "TYPE_INT should map to 'int'");
    TEST_ASSERT_STR_EQ("float", data_type_to_string(TYPE_FLOAT), "TYPE_FLOAT should map to 'float'");
    TEST_ASSERT_STR_EQ("string", data_type_to_string(TYPE_STRING), "TYPE_STRING should map to 'string'");
    TEST_ASSERT_STR_EQ("bool", data_type_to_string(TYPE_BOOL), "TYPE_BOOL should map to 'bool'");
    TEST_ASSERT_STR_EQ("void", data_type_to_string(TYPE_VOID), "TYPE_VOID should map to 'void'");
    TEST_ASSERT_STR_EQ("unknown", data_type_to_string(TYPE_UNKNOWN), "TYPE_UNKNOWN should map to 'unknown'");

    TEST_ASSERT_STR_EQ("VARIABLE", symbol_type_to_string(SYMBOL_VARIABLE), "SYMBOL_VARIABLE should map to 'VARIABLE'");
    TEST_ASSERT_STR_EQ("FUNCTION", symbol_type_to_string(SYMBOL_FUNCTION), "SYMBOL_FUNCTION should map to 'FUNCTION'");
    TEST_ASSERT_STR_EQ("PARAMETER", symbol_type_to_string(SYMBOL_PARAMETER), "SYMBOL_PARAMETER should map to 'PARAMETER'");
}

// Add this test suite to the runner
void run_semantic_tests(void) {
    run_suite_symbol_table_creation();
    run_suite_symbol_creation();
    run_suite_symbol_table_add_lookup();
    run_suite_semantic_analyzer_creation();
    run_suite_scope_management();
    run_suite_type_inference();
    run_suite_binary_operation_type_checking();
    run_suite_semantic_analysis_simple();
    run_suite_data_type_utility();
}