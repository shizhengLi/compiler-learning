#include "../src/lexer/lexer.h"
#include "../src/parser/parser.h"
#include "../src/semantic/semantic.h"
#include "../src/codegen/codegen.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int test_count = 0;
int passed_tests = 0;

#define TEST_ASSERT(condition, message) do { \
    test_count++; \
    if (!(condition)) { \
        printf("  ✗ %s\n", message); \
    } else { \
        passed_tests++; \
    } \
} while(0)

#define TEST_ASSERT_STR_EQ(expected, actual, message) do { \
    test_count++; \
    if (strcmp(expected, actual) != 0) { \
        printf("  ✗ %s (expected: '%s', actual: '%s')\n", message, expected, actual); \
    } else { \
        passed_tests++; \
    } \
} while(0)

#define TEST_ASSERT_NOT_NULL(ptr, message) do { \
    test_count++; \
    if ((ptr) == NULL) { \
        printf("  ✗ %s (pointer is NULL)\n", message); \
    } else { \
        passed_tests++; \
    } \
} while(0)

int test_simple_expression_compilation(void) {
    printf("Test 1: Simple Expression Compilation\n");

    // Test: Compile "5 + 3"
    const char* source = "5 + 3";
    const char* output_file = "test_expression.asm";

    // Lexer phase
    Lexer* lexer = lexer_create(source);
    TEST_ASSERT_NOT_NULL(lexer, "Lexer should be created");

    // Parser phase
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* ast = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "Parser should generate AST");
    TEST_ASSERT(ast->type == NODE_BINARY_EXPRESSION, "Should be binary expression");

    // Semantic analysis phase
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");

    bool semantic_result = semantic_analyze(ast, analyzer);
    TEST_ASSERT(semantic_result && !analyzer->had_error, "Semantic analysis should succeed");

    // Code generation phase
    CodeGenerator* generator = code_generator_create(analyzer->current_scope);
    TEST_ASSERT_NOT_NULL(generator, "Code generator should be created");

    CodeGenResult codegen_result = code_generator_generate(generator, ast, output_file);
    TEST_ASSERT(codegen_result == CODEGEN_SUCCESS, "Code generation should succeed");

    // Verify assembly file was created and contains expected content
    TEST_ASSERT(access(output_file, F_OK) == 0, "Assembly file should be created");

    FILE* file = fopen(output_file, "r");
    if (file) {
        char buffer[256];
        int found_main = 0;
        int found_add = 0;
        int found_mov = 0;

        while (fgets(buffer, sizeof(buffer), file)) {
            if (strstr(buffer, "_main:")) found_main = 1;
            if (strstr(buffer, "add")) found_add = 1;
            if (strstr(buffer, "mov")) found_mov = 1;
        }
        fclose(file);

        TEST_ASSERT(found_main, "Assembly should contain main function");
        TEST_ASSERT(found_add, "Assembly should contain addition instruction");
        TEST_ASSERT(found_mov, "Assembly should contain move instruction");

        remove(output_file); // Clean up
    }

    // Cleanup
    code_generator_free(generator);
    semantic_analyzer_free(analyzer);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);

    return 1;
}

int test_variable_declaration_compilation(void) {
    printf("Test 2: Variable Declaration Compilation\n");

    // Test: Compile "int x = 42"
    const char* source = "int x = 42";
    const char* output_file = "test_variable.asm";

    // Lexer phase
    Lexer* lexer = lexer_create(source);
    TEST_ASSERT_NOT_NULL(lexer, "Lexer should be created");

    // Parser phase
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* ast = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "Parser should generate AST");

    // Semantic analysis phase
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");

    bool semantic_result = semantic_analyze(ast, analyzer);
    TEST_ASSERT(semantic_result && !analyzer->had_error, "Semantic analysis should succeed");

    // Code generation phase
    CodeGenerator* generator = code_generator_create(analyzer->current_scope);
    TEST_ASSERT_NOT_NULL(generator, "Code generator should be created");

    CodeGenResult codegen_result = code_generator_generate(generator, ast, output_file);
    TEST_ASSERT(codegen_result == CODEGEN_SUCCESS, "Code generation should succeed");

    // Verify assembly file was created
    TEST_ASSERT(access(output_file, F_OK) == 0, "Assembly file should be created");

    FILE* file = fopen(output_file, "r");
    if (file) {
        char buffer[256];
        int found_main = 0;
        int found_stack = 0;

        while (fgets(buffer, sizeof(buffer), file)) {
            if (strstr(buffer, "_main:")) found_main = 1;
            if (strstr(buffer, "sub") && strstr(buffer, "rsp")) found_stack = 1;
        }
        fclose(file);

        TEST_ASSERT(found_main, "Assembly should contain main function");
        TEST_ASSERT(found_stack, "Assembly should contain stack allocation");

        remove(output_file); // Clean up
    }

    // Cleanup
    code_generator_free(generator);
    semantic_analyzer_free(analyzer);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);

    return 1;
}

int test_complex_expression_compilation(void) {
    printf("Test 3: Complex Expression Compilation\n");

    // Test: Compile "1 + 2 * 3"
    const char* source = "1 + 2 * 3";
    const char* output_file = "test_complex.asm";

    // Lexer phase
    Lexer* lexer = lexer_create(source);
    TEST_ASSERT_NOT_NULL(lexer, "Lexer should be created");

    // Parser phase
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* ast = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "Parser should generate AST");
    TEST_ASSERT(ast->type == NODE_BINARY_EXPRESSION, "Should be binary expression");

    // Semantic analysis phase
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");

    bool semantic_result = semantic_analyze(ast, analyzer);
    TEST_ASSERT(semantic_result && !analyzer->had_error, "Semantic analysis should succeed");

    // Code generation phase
    CodeGenerator* generator = code_generator_create(analyzer->current_scope);
    TEST_ASSERT_NOT_NULL(generator, "Code generator should be created");

    CodeGenResult codegen_result = code_generator_generate(generator, ast, output_file);
    TEST_ASSERT(codegen_result == CODEGEN_SUCCESS, "Code generation should succeed");

    // Verify assembly file was created
    TEST_ASSERT(access(output_file, F_OK) == 0, "Assembly file should be created");

    FILE* file = fopen(output_file, "r");
    if (file) {
        char buffer[256];
        int found_main = 0;
        int operation_count = 0;

        while (fgets(buffer, sizeof(buffer), file)) {
            if (strstr(buffer, "_main:")) found_main = 1;
            if (strstr(buffer, "add") || strstr(buffer, "imul")) operation_count++;
        }
        fclose(file);

        TEST_ASSERT(found_main, "Assembly should contain main function");
        TEST_ASSERT(operation_count >= 2, "Assembly should contain multiple operations");

        remove(output_file); // Clean up
    }

    // Cleanup
    code_generator_free(generator);
    semantic_analyzer_free(analyzer);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);

    return 1;
}

int test_compiler_error_handling(void) {
    printf("Test 4: Compiler Error Handling\n");

    // Test: Compile invalid syntax "int x = ;"
    const char* source = "int x = ;";

    // Lexer phase
    Lexer* lexer = lexer_create(source);
    TEST_ASSERT_NOT_NULL(lexer, "Lexer should be created");

    // Parser phase
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* ast = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "Parser should generate AST (may be error node)");
    TEST_ASSERT(parser_had_error(parser) || ast->type == NODE_ERROR, "Parser should detect error");

    // Cleanup
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);

    return 1;
}

int test_full_pipeline_integration(void) {
    printf("Test 5: Full Pipeline Integration\n");

    // Test the complete compilation pipeline for a valid program
    const char* source = "42";
    const char* output_file = "test_pipeline.asm";

    // Complete compilation pipeline
    Lexer* lexer = lexer_create(source);
    TEST_ASSERT_NOT_NULL(lexer, "Lexer phase");

    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser phase");

    ASTNode* ast = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "AST generation");

    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analysis");

    bool semantic_result = semantic_analyze(ast, analyzer);
    TEST_ASSERT(semantic_result && !analyzer->had_error, "Semantic validation");

    CodeGenerator* generator = code_generator_create(analyzer->current_scope);
    TEST_ASSERT_NOT_NULL(generator, "Code generation");

    CodeGenResult codegen_result = code_generator_generate(generator, ast, output_file);
    TEST_ASSERT(codegen_result == CODEGEN_SUCCESS, "Code generation success");

    // Verify the final output
    TEST_ASSERT(access(output_file, F_OK) == 0, "Final assembly file exists");

    // Check if assembly can be assembled (basic validation)
    FILE* file = fopen(output_file, "r");
    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fclose(file);
        TEST_ASSERT(size > 50, "Assembly file should have reasonable content size");
        remove(output_file);
    }

    // Cleanup all components
    code_generator_free(generator);
    semantic_analyzer_free(analyzer);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);

    return 1;
}

int main(void) {
    printf("=== INTEGRATION TEST SUITE ===\n");
    printf("Testing complete compiler pipeline from source to assembly\n\n");

    test_simple_expression_compilation();
    test_variable_declaration_compilation();
    test_complex_expression_compilation();
    test_compiler_error_handling();
    test_full_pipeline_integration();

    printf("\n=== INTEGRATION TEST RESULTS ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", test_count - passed_tests);
    printf("Success rate: %.1f%%\n", test_count > 0 ? (float)passed_tests / test_count * 100.0 : 0.0);

    if (passed_tests == test_count) {
        printf("🎉 ALL INTEGRATION TESTS PASSED! 🎉\n");
        printf("✅ Complete compiler pipeline working end-to-end\n");
        printf("✅ Source → Lexer → Parser → Semantic Analyzer → Code Generator → Assembly\n");
        return 0;
    } else {
        printf("❌ SOME INTEGRATION TESTS FAILED ❌\n");
        return 1;
    }
}