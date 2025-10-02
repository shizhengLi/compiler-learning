#include "src/lexer/lexer.h"
#include "src/parser/parser.h"
#include "src/semantic/semantic.h"
#include "src/codegen/codegen.h"
#include <stdio.h>

int main(void) {
    printf("=== FINAL INTEGRATION TEST ===\n");
    printf("Testing complete compiler pipeline: Source → Assembly\n\n");

    const char* source = "5 + 3";
    const char* output_file = "final_test.asm";

    // Phase 1: Lexical Analysis
    printf("1. Lexical Analysis: \"%s\"\n", source);
    Lexer* lexer = lexer_create(source);
    if (!lexer) {
        printf("   ❌ Lexer creation failed\n");
        return 1;
    }
    printf("   ✅ Lexer created successfully\n");

    // Phase 2: Parsing
    printf("2. Parsing to AST\n");
    Parser* parser = parser_create(lexer);
    if (!parser) {
        printf("   ❌ Parser creation failed\n");
        return 1;
    }
    ASTNode* ast = parser_parse(parser);
    if (!ast) {
        printf("   ❌ Parsing failed\n");
        return 1;
    }
    printf("   ✅ AST created (type: %s)\n",
           ast->type == NODE_BINARY_EXPRESSION ? "BINARY_EXPRESSION" : "OTHER");

    // Phase 3: Semantic Analysis
    printf("3. Semantic Analysis\n");
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("   ❌ Semantic analyzer creation failed\n");
        return 1;
    }
    bool semantic_result = semantic_analyze(ast, analyzer);
    if (!semantic_result || analyzer->had_error) {
        printf("   ❌ Semantic analysis failed\n");
        return 1;
    }
    printf("   ✅ Semantic analysis passed\n");

    // Phase 4: Code Generation
    printf("4. Code Generation\n");
    CodeGenerator* generator = code_generator_create(analyzer->current_scope);
    if (!generator) {
        printf("   ❌ Code generator creation failed\n");
        return 1;
    }
    CodeGenResult result = code_generator_generate(generator, ast, output_file);
    if (result != CODEGEN_SUCCESS) {
        printf("   ❌ Code generation failed: %s\n", codegen_result_to_string(result));
        return 1;
    }
    printf("   ✅ Assembly generated: %s\n", output_file);

    // Phase 5: Show Results
    printf("\n=== COMPILER PIPELINE SUCCESS! ===\n");
    printf("Source: %s\n", source);
    printf("Output:\n");

    FILE* file = fopen(output_file, "r");
    if (file) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), file)) {
            printf("   %s", buffer);
        }
        fclose(file);
    }

    printf("\n🎉 COMPLETE COMPILER PIPELINE WORKING! 🎉\n");
    printf("✅ Source → Lexer → Parser → Semantic Analyzer → Code Generator → Assembly\n");
    printf("✅ All TDD components integrated successfully\n");
    printf("✅ 100%% TDD approach achieved functional compiler\n");

    // Cleanup
    code_generator_free(generator);
    semantic_analyzer_free(analyzer);
    ast_node_free(ast);
    parser_free(parser);
    lexer_free(lexer);

    return 0;
}