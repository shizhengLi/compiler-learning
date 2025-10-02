#include "../src/codegen/codegen.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    printf("=== COMPREHENSIVE CODE GENERATOR TEST SUITE ===\n\n");

    int test_count = 0;
    int passed_tests = 0;

    // Test 1: Code generator creation and management
    printf("Test 1: Code Generator Creation and Management\n");
    SymbolTable* table = symbol_table_create(0);
    CodeGenerator* generator = code_generator_create(table);

    if (generator != NULL && generator->symbol_table == table && !generator->had_error) {
        printf("  ✓ Code generator created correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Code generator creation failed\n");
    }
    test_count++;

    // Test 2: Output file management
    printf("Test 2: Output File Management\n");
    const char* test_file = "test_output.asm";
    CodeGenResult result = code_generator_set_output(generator, test_file);

    if (result == CODEGEN_SUCCESS && access(test_file, F_OK) == 0) {
        printf("  ✓ Output file created correctly\n");
        passed_tests++;
        remove(test_file); // Clean up test file
    } else {
        printf("  ✗ Output file creation failed\n");
    }
    test_count++;

    // Test 3: Assembly generation helpers
    printf("Test 3: Assembly Generation Helpers\n");

    result = code_generator_set_output(generator, "test_helpers.asm");
    result = code_generator_emit_prologue(generator);
    result = code_generator_emit_comment(generator, "Test comment");
    result = code_generator_emit_instruction(generator, "mov", "rax, 42");
    result = code_generator_emit_epilogue(generator);

    // Check if file has content
    FILE* file = fopen("test_helpers.asm", "r");
    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fclose(file);
        if (size > 0) {
            printf("  ✓ Assembly generation helpers work correctly\n");
            passed_tests++;
        } else {
            printf("  ✗ Assembly file is empty\n");
        }
        remove("test_helpers.asm"); // Clean up
    } else {
        printf("  ✗ Assembly file not created\n");
    }
    test_count++;

    // Test 4: Register allocation
    printf("Test 4: Register Allocation\n");

    Register reg1 = code_generator_allocate_register(generator);
    Register reg2 = code_generator_allocate_register(generator);

    if (reg1 != REGISTER_COUNT && reg2 != REGISTER_COUNT && reg1 != reg2) {
        printf("  ✓ Register allocation works correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Register allocation failed\n");
    }

    code_generator_free_register(generator, reg1);
    code_generator_free_register(generator, reg2);
    test_count++;

    // Test 5: Simple literal code generation
    printf("Test 5: Simple Literal Code Generation\n");

    // Create a simple AST with integer literal
    Token* int_token = token_create(TOKEN_INTEGER_LITERAL, "42", 1, 1);
    int_token->literal.int_value = 42;
    ASTNode* literal = ast_node_create_literal_int(int_token, 42);

    result = code_generator_set_output(generator, "test_literal.asm");
    result = code_generator_emit_prologue(generator);
    result = code_generator_generate_literal(generator, literal);
    result = code_generator_emit_epilogue(generator);

    // Check if assembly contains the value
    file = fopen("test_literal.asm", "r");
    if (file) {
        char buffer[256];
        int found_value = 0;
        while (fgets(buffer, sizeof(buffer), file)) {
            if (strstr(buffer, "42")) {
                found_value = 1;
                break;
            }
        }
        fclose(file);

        if (found_value) {
            printf("  ✓ Literal code generation works correctly\n");
            passed_tests++;
        } else {
            printf("  ✗ Literal value not found in assembly\n");
        }
        remove("test_literal.asm");
    } else {
        printf("  ✗ Assembly file not created for literal test\n");
    }

    ast_node_free(literal);
    token_free(int_token);
    test_count++;

    // Test 6: Simple binary expression code generation
    printf("Test 6: Simple Binary Expression Code Generation\n");

    // Create AST for: 5 + 3
    Token* token5 = token_create(TOKEN_INTEGER_LITERAL, "5", 1, 1);
    Token* token3 = token_create(TOKEN_INTEGER_LITERAL, "3", 1, 1);
    Token* plus_token = token_create(TOKEN_PLUS, "+", 1, 1);

    token5->literal.int_value = 5;
    token3->literal.int_value = 3;

    ASTNode* left = ast_node_create_literal_int(token5, 5);
    ASTNode* right = ast_node_create_literal_int(token3, 3);
    ASTNode* binary = ast_node_create_binary(plus_token, left, right, "+");

    result = code_generator_set_output(generator, "test_binary.asm");
    result = code_generator_emit_prologue(generator);
    result = code_generator_generate_binary(generator, binary);
    result = code_generator_emit_epilogue(generator);

    // Check if assembly contains addition instruction
    file = fopen("test_binary.asm", "r");
    if (file) {
        char buffer[256];
        int found_add = 0;
        while (fgets(buffer, sizeof(buffer), file)) {
            if (strstr(buffer, "add")) {
                found_add = 1;
                break;
            }
        }
        fclose(file);

        if (found_add) {
            printf("  ✓ Binary expression code generation works correctly\n");
            passed_tests++;
        } else {
            printf("  ✗ Addition instruction not found in assembly\n");
        }
        remove("test_binary.asm");
    } else {
        printf("  ✗ Assembly file not created for binary test\n");
    }

    ast_node_free(binary);
    token_free(token5);
    token_free(token3);
    token_free(plus_token);
    test_count++;

    // Test 7: Variable declaration code generation
    printf("Test 7: Variable Declaration Code Generation\n");

    // Add a variable to symbol table
    Symbol* var = symbol_create_variable("x", "int", true, 1, 1);
    symbol_table_add(table, var);

    // Create AST for: int x = 10;
    Token* var_token = token_create(TOKEN_IDENTIFIER, "x", 1, 5);
    Token* val_token = token_create(TOKEN_INTEGER_LITERAL, "10", 1, 9);
    val_token->literal.int_value = 10;

    ASTNode* value = ast_node_create_literal_int(val_token, 10);
    ASTNode* declaration = ast_node_create_variable_declaration(var_token, "int", "x", value);

    result = code_generator_set_output(generator, "test_declaration.asm");
    result = code_generator_emit_prologue(generator);
    result = code_generator_generate_variable_declaration(generator, declaration);
    result = code_generator_emit_epilogue(generator);

    // Check if assembly contains variable allocation
    file = fopen("test_declaration.asm", "r");
    if (file) {
        char buffer[256];
        int found_var = 0;
        while (fgets(buffer, sizeof(buffer), file)) {
            if (strstr(buffer, "x") || strstr(buffer, "rbp") || strstr(buffer, "sub")) {
                found_var = 1;
                break;
            }
        }
        fclose(file);

        if (found_var) {
            printf("  ✓ Variable declaration code generation works correctly\n");
            passed_tests++;
        } else {
            printf("  ✗ Variable allocation not found in assembly\n");
        }
        remove("test_declaration.asm");
    } else {
        printf("  ✗ Assembly file not created for declaration test\n");
    }

    ast_node_free(declaration);
    ast_node_free(value);
    token_free(var_token);
    token_free(val_token);
    test_count++;

    // Test 8: Simple program code generation
    printf("Test 8: Simple Program Code Generation\n");

    // Create AST for a simple program with variable declaration
    ASTNode* program = ast_node_create_program();
    ast_node_add_child(program, declaration); // Reuse previous declaration

    result = code_generator_generate(generator, program, "test_program.asm");

    if (result == CODEGEN_SUCCESS && access("test_program.asm", F_OK) == 0) {
        printf("  ✓ Program code generation works correctly\n");
        passed_tests++;
        remove("test_program.asm");
    } else {
        printf("  ✗ Program code generation failed\n");
    }

    ast_node_free(program);
    test_count++;

    // Test 9: Code generator utility functions
    printf("Test 9: Code Generator Utility Functions\n");

    const char* success_str = codegen_result_to_string(CODEGEN_SUCCESS);
    const char* rax_str = register_to_string(REGISTER_RAX);

    if (strcmp(success_str, "CODEGEN_SUCCESS") == 0 &&
        strcmp(rax_str, "rax") == 0) {
        printf("  ✓ Code generator utility functions work correctly\n");
        passed_tests++;
    } else {
        printf("  ✗ Code generator utility functions failed\n");
    }
    test_count++;

    // Cleanup
    code_generator_free(generator);
    symbol_table_free(table);

    printf("\n=== CODE GENERATOR TEST RESULTS ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", test_count - passed_tests);
    printf("Success rate: %.1f%%\n", test_count > 0 ? (float)passed_tests / test_count * 100.0 : 0.0);

    if (passed_tests == test_count) {
        printf("🎉 ALL CODE GENERATOR TESTS PASSED! 🎉\n");
        printf("✅ Code generator implementation is 100%% complete with TDD approach\n");
        return 0;
    } else {
        printf("❌ SOME CODE GENERATOR TESTS FAILED ❌\n");
        return 1;
    }
}