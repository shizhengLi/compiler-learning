#include "src/codegen/codegen.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== DEBUGGING CODE GENERATOR TEST 6 ===\n");

    SymbolTable* table = symbol_table_create(0);
    CodeGenerator* generator = code_generator_create(table);

    printf("Step 1: Created generator\n");

    // Create AST for: 5 + 3
    Token* token5 = token_create(TOKEN_INTEGER_LITERAL, "5", 1, 1);
    Token* token3 = token_create(TOKEN_INTEGER_LITERAL, "3", 1, 1);
    Token* plus_token = token_create(TOKEN_PLUS, "+", 1, 1);

    printf("Step 2: Created tokens\n");

    token5->literal.int_value = 5;
    token3->literal.int_value = 3;

    printf("Step 3: Set token values\n");

    ASTNode* left = ast_node_create_literal_int(token5, 5);
    ASTNode* right = ast_node_create_literal_int(token3, 3);
    ASTNode* binary = ast_node_create_binary(plus_token, left, right, "+");

    printf("Step 4: Created AST nodes\n");

    CodeGenResult result = code_generator_set_output(generator, "debug_binary.asm");
    printf("Step 5: Set output file: %s\n", codegen_result_to_string(result));

    result = code_generator_emit_prologue(generator);
    printf("Step 6: Emitted prologue: %s\n", codegen_result_to_string(result));

    result = code_generator_generate_binary(generator, binary);
    printf("Step 7: Generated binary: %s\n", codegen_result_to_string(result));

    result = code_generator_emit_epilogue(generator);
    printf("Step 8: Emitted epilogue: %s\n", codegen_result_to_string(result));

    printf("✅ Test completed successfully!\n");

    // Cleanup
    ast_node_free(binary);
    token_free(token5);
    token_free(token3);
    token_free(plus_token);
    code_generator_free(generator);
    symbol_table_free(table);

    return 0;
}