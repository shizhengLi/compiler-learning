#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/semantic/semantic.h"
#include "src/common/common.h"
#include "src/lexer/lexer.h"
#include "src/parser/parser.h"
#include "src/codegen/codegen.h"

int main() {
    printf("=== DEBUG CODEGEN TEST ===\n");

    // Test 1: Simple literal
    printf("Test 1: Simple literal '42'\n");

    const char* source = "42";
    Lexer* lexer = lexer_create(source);
    Parser* parser = parser_create(lexer);
    ASTNode* ast = parser_parse(parser);

    if (!ast) {
        printf("  ✗ Failed to parse '42'\n");
        return 1;
    }

    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    bool semantic_result = semantic_analyze(ast, analyzer);

    if (!semantic_result) {
        printf("  ✗ Semantic analysis failed\n");
        return 1;
    }

    CodeGenerator* generator = code_generator_create(analyzer->current_scope);
    if (!generator) {
        printf("  ✗ Failed to create code generator\n");
        return 1;
    }

    CodeGenResult result = code_generator_generate(generator, ast, "test_literal.asm");
    printf("  Codegen result: %s\n", codegen_result_to_string(result));

    if (result == CODEGEN_SUCCESS) {
        printf("  ✓ Code generation succeeded\n");
        // Show generated assembly
        FILE* file = fopen("test_literal.asm", "r");
        if (file) {
            printf("  Generated assembly:\n");
            char line[256];
            while (fgets(line, sizeof(line), file)) {
                printf("    %s", line);
            }
            fclose(file);
        }
    } else {
        printf("  ✗ Code generation failed: %s\n", codegen_result_to_string(result));
        if (generator->last_error[0] != '\0') {
            printf("    Error: %s\n", generator->last_error);
        }
    }

    // Test 2: Simple binary expression
    printf("\nTest 2: Binary expression '5 + 3'\n");

    const char* source2 = "5 + 3";
    Lexer* lexer2 = lexer_create(source2);
    Parser* parser2 = parser_create(lexer2);
    ASTNode* ast2 = parser_parse(parser2);

    if (!ast2) {
        printf("  ✗ Failed to parse '5 + 3'\n");
        return 1;
    }

    SemanticAnalyzer* analyzer2 = semantic_analyzer_create();
    bool semantic_result2 = semantic_analyze(ast2, analyzer2);

    if (!semantic_result2) {
        printf("  ✗ Semantic analysis failed\n");
        return 1;
    }

    CodeGenerator* generator2 = code_generator_create(analyzer2->current_scope);
    if (!generator2) {
        printf("  ✗ Failed to create code generator\n");
        return 1;
    }

    CodeGenResult result2 = code_generator_generate(generator2, ast2, "test_binary.asm");
    printf("  Codegen result: %s\n", codegen_result_to_string(result2));

    if (result2 == CODEGEN_SUCCESS) {
        printf("  ✓ Code generation succeeded\n");
        // Show generated assembly
        FILE* file2 = fopen("test_binary.asm", "r");
        if (file2) {
            printf("  Generated assembly:\n");
            char line[256];
            while (fgets(line, sizeof(line), file2)) {
                printf("    %s", line);
            }
            fclose(file2);
        }
    } else {
        printf("  ✗ Code generation failed: %s\n", codegen_result_to_string(result2));
        if (generator2->last_error[0] != '\0') {
            printf("    Error: %s\n", generator2->last_error);
        }
    }

    // Cleanup
    lexer_free(lexer);
    parser_free(parser);
    ast_node_free(ast);
    semantic_analyzer_free(analyzer);
    code_generator_free(generator);

    lexer_free(lexer2);
    parser_free(parser2);
    ast_node_free(ast2);
    semantic_analyzer_free(analyzer2);
    code_generator_free(generator2);

    return 0;
}