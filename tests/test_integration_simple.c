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
        printf("  ✓ %s\n", message); \
    } \
} while(0)

#define TEST_ASSERT_NOT_NULL(ptr, message) do { \
    test_count++; \
    if ((ptr) == NULL) { \
        printf("  ✗ %s (pointer is NULL)\n", message); \
    } else { \
        passed_tests++; \
        printf("  ✓ %s\n", message); \
    } \
} while(0)

int test_expression_to_assembly_pipeline(void) {
    printf("Test 1: Expression to Assembly Pipeline\n");

    // Test: Complete pipeline for "5 + 3"
    const char* source = "5 + 3";
    const char* output_file = "integration_test.asm";

    // Phase 1: Lexical Analysis
    printf("  Phase 1: Lexical Analysis\n");
    Lexer* lexer = lexer_create(source);
    TEST_ASSERT_NOT_NULL(lexer, "Lexer should be created");

    // Test that lexer produces correct tokens
    Token* token = lexer_next_token(lexer);
    TEST_ASSERT(token && token->type == TOKEN_INTEGER_LITERAL && token->literal.int_value == 5,
                "Should parse integer literal 5");

    token = lexer_next_token(lexer);
    TEST_ASSERT(token && token->type == TOKEN_PLUS, "Should parse plus operator");

    token = lexer_next_token(lexer);
    TEST_ASSERT(token && token->type == TOKEN_INTEGER_LITERAL && token->literal.int_value == 3,
                "Should parse integer literal 3");

    // Phase 2: Parsing
    printf("  Phase 2: Parsing\n");
    // Create new lexer for parser since we don't have reset function
    lexer_free(lexer);
    lexer = lexer_create(source);
    Parser* parser = parser_create(lexer);
    TEST_ASSERT_NOT_NULL(parser, "Parser should be created");

    ASTNode* ast = parser_parse(parser);
    TEST_ASSERT_NOT_NULL(ast, "Parser should generate AST");
    TEST_ASSERT(ast->type == NODE_BINARY_EXPRESSION, "Should create binary expression AST");

    // Phase 3: Semantic Analysis
    printf("  Phase 3: Semantic Analysis\n");
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    TEST_ASSERT_NOT_NULL(analyzer, "Semantic analyzer should be created");

    bool semantic_result = semantic_analyze(ast, analyzer);
    TEST_ASSERT(semantic_result && !analyzer->had_error, "Semantic analysis should succeed");

    // Phase 4: Code Generation
    printf("  Phase 4: Code Generation\n");
    CodeGenerator* generator = code_generator_create(analyzer->current_scope);
    TEST_ASSERT_NOT_NULL(generator, "Code generator should be created");

    CodeGenResult codegen_result = code_generator_generate(generator, ast, output_file);
    TEST_ASSERT(codegen_result == CODEGEN_SUCCESS, "Code generation should succeed");

    // Phase 5: Verify Output
    printf("  Phase 5: Output Verification\n");
    TEST_ASSERT(access(output_file, F_OK) == 0, "Assembly file should be created");

    FILE* file = fopen(output_file, "r");
    if (file) {
        char buffer[256];
        int found_main = 0;
        int found_add = 0;
        int found_prologue = 0;
        int found_epilogue = 0;

        while (fgets(buffer, sizeof(buffer), file)) {
            if (strstr(buffer, "_main:")) found_main = 1;
            if (strstr(buffer, "add")) found_add = 1;
            if (strstr(buffer, "push    rbp")) found_prologue = 1;
            if (strstr(buffer, "ret")) found_epilogue = 1;
        }
        fclose(file);

        TEST_ASSERT(found_main, "Assembly should contain main function");
        TEST_ASSERT(found_add, "Assembly should contain addition");
        TEST_ASSERT(found_prologue, "Assembly should contain function prologue");
        TEST_ASSERT(found_epilogue, "Assembly should contain function epilogue");

        // Display generated assembly for verification
        printf("  Generated assembly:\n");
        file = fopen(output_file, "r");
        if (file) {
            while (fgets(buffer, sizeof(buffer), file)) {
                printf("    %s", buffer);
            }
            fclose(file);
        }

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

int test_literal_pipeline(void) {
    printf("\nTest 2: Literal Pipeline\n");

    // Test: Complete pipeline for "42"
    const char* source = "42";
    const char* output_file = "literal_test.asm";

    // Complete pipeline
    Lexer* lexer = lexer_create(source);
    Parser* parser = parser_create(lexer);
    ASTNode* ast = parser_parse(parser);
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    bool semantic_result = semantic_analyze(ast, analyzer);
    CodeGenerator* generator = code_generator_create(analyzer->current_scope);
    CodeGenResult codegen_result = code_generator_generate(generator, ast, output_file);

    TEST_ASSERT_NOT_NULL(lexer, "Lexer created");
    TEST_ASSERT_NOT_NULL(parser, "Parser created");
    TEST_ASSERT_NOT_NULL(ast, "AST created");
    TEST_ASSERT(semantic_result && !analyzer->had_error, "Semantic analysis passed");
    TEST_ASSERT_NOT_NULL(generator, "Code generator created");
    TEST_ASSERT(codegen_result == CODEGEN_SUCCESS, "Code generation successful");
    TEST_ASSERT(access(output_file, F_OK) == 0, "Assembly file created");

    // Verify assembly contains the literal value
    FILE* file = fopen(output_file, "r");
    if (file) {
        char buffer[256];
        int found_42 = 0;
        while (fgets(buffer, sizeof(buffer), file)) {
            if (strstr(buffer, "42")) found_42 = 1;
        }
        fclose(file);
        TEST_ASSERT(found_42, "Assembly should contain the literal value 42");
        remove(output_file);
    }

    // Cleanup
    code_generator_free(generator);
    semantic_analyzer_free(analyzer);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);

    return 1;
}

int test_error_handling_pipeline(void) {
    printf("\nTest 3: Error Handling Pipeline\n");

    // Test: Error handling for invalid input
    const char* source = ""; // Empty input

    Lexer* lexer = lexer_create(source);
    Parser* parser = parser_create(lexer);
    ASTNode* ast = parser_parse(parser);

    TEST_ASSERT_NOT_NULL(lexer, "Lexer should handle empty input");
    TEST_ASSERT_NOT_NULL(parser, "Parser should handle empty input");
    TEST_ASSERT_NOT_NULL(ast, "Parser should return some AST even for empty input");

    // Cleanup
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);

    return 1;
}

int main(void) {
    printf("=== INTEGRATION TEST SUITE ===\n");
    printf("Testing complete compiler pipeline integration\n");
    printf("Source → Lexer → Parser → Semantic Analyzer → Code Generator → Assembly\n\n");

    test_expression_to_assembly_pipeline();
    test_literal_pipeline();
    test_error_handling_pipeline();

    printf("\n=== INTEGRATION TEST RESULTS ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", test_count - passed_tests);
    printf("Success rate: %.1f%%\n", test_count > 0 ? (float)passed_tests / test_count * 100.0 : 0.0);

    if (passed_tests == test_count) {
        printf("\n🎉 ALL INTEGRATION TESTS PASSED! 🎉\n");
        printf("✅ Complete compiler pipeline is working!\n");
        printf("✅ Successfully compiles source code to assembly\n");
        printf("✅ All compiler components integrated correctly\n");
        return 0;
    } else {
        printf("\n❌ SOME INTEGRATION TESTS FAILED ❌\n");
        return 1;
    }
}