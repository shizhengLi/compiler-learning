#ifndef CODEGEN_H
#define CODEGEN_H

#include "../semantic/semantic.h"
#include "../parser/parser.h"

// Code generation result types
typedef enum {
    CODEGEN_SUCCESS,
    CODEGEN_ERROR_NULL_ANALYZER,
    CODEGEN_ERROR_NULL_AST,
    CODEGEN_ERROR_UNSUPPORTED_NODE,
    CODEGEN_ERROR_SYMBOL_NOT_FOUND,
    CODEGEN_ERROR_TYPE_MISMATCH,
    CODEGEN_ERROR_INVALID_EXPRESSION
} CodeGenResult;

// Register allocation
typedef enum {
    REGISTER_RAX,
    REGISTER_RBX,
    REGISTER_RCX,
    REGISTER_RDX,
    REGISTER_RSI,
    REGISTER_RDI,
    REGISTER_R8,
    REGISTER_R9,
    REGISTER_R10,
    REGISTER_R11,
    REGISTER_R12,
    REGISTER_R13,
    REGISTER_R14,
    REGISTER_R15,
    REGISTER_RBP,
    REGISTER_RSP,
    REGISTER_COUNT
} Register;

// Code generator structure
typedef struct CodeGenerator {
    SymbolTable* symbol_table;
    FILE* output_file;
    int had_error;
    char last_error[256];
    int label_counter;
    int stack_offset;
    Register used_registers[REGISTER_COUNT];
    int temp_var_counter;
} CodeGenerator;

// Main code generator functions
CodeGenerator* code_generator_create(SymbolTable* symbol_table);
void code_generator_free(CodeGenerator* generator);
CodeGenResult code_generator_generate(CodeGenerator* generator, ASTNode* ast, const char* output_filename);
CodeGenResult code_generator_set_output(CodeGenerator* generator, const char* output_filename);

// Expression code generation
CodeGenResult code_generator_generate_expression(CodeGenerator* generator, ASTNode* node);
CodeGenResult code_generator_generate_literal(CodeGenerator* generator, ASTNode* node);
CodeGenResult code_generator_generate_identifier(CodeGenerator* generator, ASTNode* node);
CodeGenResult code_generator_generate_binary(CodeGenerator* generator, ASTNode* node);
CodeGenResult code_generator_generate_unary(CodeGenerator* generator, ASTNode* node);
CodeGenResult code_generator_generate_assignment(CodeGenerator* generator, ASTNode* node);

// Statement code generation
CodeGenResult code_generator_generate_variable_declaration(CodeGenerator* generator, ASTNode* node);
CodeGenResult code_generator_generate_program(CodeGenerator* generator, ASTNode* node);

// Utility functions
const char* codegen_result_to_string(CodeGenResult result);
const char* register_to_string(Register reg);
Register code_generator_allocate_register(CodeGenerator* generator);
void code_generator_free_register(CodeGenerator* generator, Register reg);
void code_generator_error(CodeGenerator* generator, const char* format, ...);

// Assembly generation helpers
CodeGenResult code_generator_emit_prologue(CodeGenerator* generator);
CodeGenResult code_generator_emit_epilogue(CodeGenerator* generator);
CodeGenResult code_generator_emit_label(CodeGenerator* generator, const char* label);
CodeGenResult code_generator_emit_instruction(CodeGenerator* generator, const char* instruction, const char* operands);
CodeGenResult code_generator_emit_comment(CodeGenerator* generator, const char* comment);

// Stack management
CodeGenResult code_generator_push_stack(CodeGenerator* generator, int size);
CodeGenResult code_generator_pop_stack(CodeGenerator* generator, int size);

#endif // CODEGEN_H