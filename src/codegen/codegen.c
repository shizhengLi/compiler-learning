#include "codegen.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CodeGenerator* code_generator_create(SymbolTable* symbol_table) {
    if (!symbol_table) return NULL;

    CodeGenerator* generator = malloc(sizeof(CodeGenerator));
    if (!generator) return NULL;

    generator->symbol_table = symbol_table;
    generator->output_file = NULL;
    generator->had_error = 0;
    generator->last_error[0] = '\0';
    generator->label_counter = 0;
    generator->stack_offset = 0;
    generator->temp_var_counter = 0;

    // Initialize all registers as free
    for (int i = 0; i < REGISTER_COUNT; i++) {
        generator->used_registers[i] = 0;
    }

    return generator;
}

void code_generator_free(CodeGenerator* generator) {
    if (!generator) return;

    if (generator->output_file) {
        fclose(generator->output_file);
    }

    free(generator);
}

CodeGenResult code_generator_set_output(CodeGenerator* generator, const char* output_filename) {
    if (!generator || !output_filename) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    if (generator->output_file) {
        fclose(generator->output_file);
    }

    generator->output_file = fopen(output_filename, "w");
    if (!generator->output_file) {
        code_generator_error(generator, "Failed to open output file: %s", output_filename);
        return CODEGEN_ERROR_INVALID_EXPRESSION;
    }

    // Ensure immediate write to disk
    setvbuf(generator->output_file, NULL, _IONBF, 0);

    return CODEGEN_SUCCESS;
}

CodeGenResult code_generator_emit_prologue(CodeGenerator* generator) {
    if (!generator || !generator->output_file) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    fprintf(generator->output_file, "    .section .data\n");
    fprintf(generator->output_file, "    .section .text\n");
    fprintf(generator->output_file, "    .global _main\n");
    fprintf(generator->output_file, "_main:\n");
    fprintf(generator->output_file, "    push    rbp\n");
    fprintf(generator->output_file, "    mov     rbp, rsp\n");
    fflush(generator->output_file);

    return CODEGEN_SUCCESS;
}

CodeGenResult code_generator_emit_epilogue(CodeGenerator* generator) {
    if (!generator || !generator->output_file) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    fprintf(generator->output_file, "    mov     rsp, rbp\n");
    fprintf(generator->output_file, "    pop     rbp\n");
    fprintf(generator->output_file, "    ret\n");
    fflush(generator->output_file);

    return CODEGEN_SUCCESS;
}

CodeGenResult code_generator_emit_comment(CodeGenerator* generator, const char* comment) {
    if (!generator || !generator->output_file || !comment) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    fprintf(generator->output_file, "    # %s\n", comment);
    return CODEGEN_SUCCESS;
}

CodeGenResult code_generator_emit_instruction(CodeGenerator* generator, const char* instruction, const char* operands) {
    if (!generator || !generator->output_file || !instruction) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    if (operands) {
        fprintf(generator->output_file, "    %-7s %s\n", instruction, operands);
    } else {
        fprintf(generator->output_file, "    %s\n", instruction);
    }

    fflush(generator->output_file);
    return CODEGEN_SUCCESS;
}

Register code_generator_allocate_register(CodeGenerator* generator) {
    if (!generator) return REGISTER_COUNT;

    // Find first free register (excluding RBP and RSP)
    for (Register i = REGISTER_RAX; i < REGISTER_RBP; i++) {
        if (!generator->used_registers[i]) {
            generator->used_registers[i] = 1;
            return i;
        }
    }

    return REGISTER_COUNT; // No free registers
}

void code_generator_free_register(CodeGenerator* generator, Register reg) {
    if (generator && reg < REGISTER_COUNT) {
        generator->used_registers[reg] = 0;
    }
}

const char* register_to_string(Register reg) {
    switch (reg) {
        case REGISTER_RAX: return "rax";
        case REGISTER_RBX: return "rbx";
        case REGISTER_RCX: return "rcx";
        case REGISTER_RDX: return "rdx";
        case REGISTER_RSI: return "rsi";
        case REGISTER_RDI: return "rdi";
        case REGISTER_R8:  return "r8";
        case REGISTER_R9:  return "r9";
        case REGISTER_R10: return "r10";
        case REGISTER_R11: return "r11";
        case REGISTER_R12: return "r12";
        case REGISTER_R13: return "r13";
        case REGISTER_R14: return "r14";
        case REGISTER_R15: return "r15";
        case REGISTER_RBP: return "rbp";
        case REGISTER_RSP: return "rsp";
        default: return "unknown";
    }
}

const char* codegen_result_to_string(CodeGenResult result) {
    switch (result) {
        case CODEGEN_SUCCESS: return "CODEGEN_SUCCESS";
        case CODEGEN_ERROR_NULL_ANALYZER: return "CODEGEN_ERROR_NULL_ANALYZER";
        case CODEGEN_ERROR_NULL_AST: return "CODEGEN_ERROR_NULL_AST";
        case CODEGEN_ERROR_UNSUPPORTED_NODE: return "CODEGEN_ERROR_UNSUPPORTED_NODE";
        case CODEGEN_ERROR_SYMBOL_NOT_FOUND: return "CODEGEN_ERROR_SYMBOL_NOT_FOUND";
        case CODEGEN_ERROR_TYPE_MISMATCH: return "CODEGEN_ERROR_TYPE_MISMATCH";
        case CODEGEN_ERROR_INVALID_EXPRESSION: return "CODEGEN_ERROR_INVALID_EXPRESSION";
        default: return "UNKNOWN";
    }
}

void code_generator_error(CodeGenerator* generator, const char* format, ...) {
    if (!generator || !format) return;

    va_list args;
    va_start(args, format);
    vsnprintf(generator->last_error, sizeof(generator->last_error), format, args);
    va_end(args);

    generator->had_error = 1;
}

CodeGenResult code_generator_generate_literal(CodeGenerator* generator, ASTNode* node) {
    if (!generator || !node || !generator->output_file) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    if (node->type != NODE_LITERAL) {
        return CODEGEN_ERROR_UNSUPPORTED_NODE;
    }

    // Generate immediate value
    if (node->token && node->token->type == TOKEN_INTEGER_LITERAL) {
        fprintf(generator->output_file, "    mov     rax, %d\n", node->data.literal.int_value);
        return CODEGEN_SUCCESS;
    }

    return CODEGEN_ERROR_UNSUPPORTED_NODE;
}

CodeGenResult code_generator_generate_binary(CodeGenerator* generator, ASTNode* node) {
    if (!generator || !node || !generator->output_file) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    if (node->type != NODE_BINARY_EXPRESSION) {
        return CODEGEN_ERROR_UNSUPPORTED_NODE;
    }

    // Generate left operand
    CodeGenResult result = code_generator_generate_expression(generator, node->data.binary.left);
    if (result != CODEGEN_SUCCESS) return result;

    // Save left result
    fprintf(generator->output_file, "    push    rax\n");

    // Generate right operand
    result = code_generator_generate_expression(generator, node->data.binary.right);
    if (result != CODEGEN_SUCCESS) return result;

    // Pop left result into rbx
    fprintf(generator->output_file, "    pop     rbx\n");

    // Generate operation based on operator
    const char* op = node->data.binary.operator;
    if (strcmp(op, "+") == 0) {
        fprintf(generator->output_file, "    add     rax, rbx\n");
    } else if (strcmp(op, "-") == 0) {
        fprintf(generator->output_file, "    sub     rbx, rax\n");
        fprintf(generator->output_file, "    mov     rax, rbx\n");
    } else if (strcmp(op, "*") == 0) {
        fprintf(generator->output_file, "    imul    rax, rbx\n");
    } else {
        return CODEGEN_ERROR_UNSUPPORTED_NODE;
    }

    return CODEGEN_SUCCESS;
}

CodeGenResult code_generator_generate_expression(CodeGenerator* generator, ASTNode* node) {
    if (!generator || !node) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    switch (node->type) {
        case NODE_LITERAL:
            return code_generator_generate_literal(generator, node);
        case NODE_BINARY_EXPRESSION:
            return code_generator_generate_binary(generator, node);
        case NODE_IDENTIFIER:
            // TODO: Implement identifier generation
            return CODEGEN_ERROR_UNSUPPORTED_NODE;
        case NODE_UNARY_EXPRESSION:
            // TODO: Implement unary expression generation
            return CODEGEN_ERROR_UNSUPPORTED_NODE;
        default:
            return CODEGEN_ERROR_UNSUPPORTED_NODE;
    }
}

CodeGenResult code_generator_generate_variable_declaration(CodeGenerator* generator, ASTNode* node) {
    if (!generator || !node || !generator->output_file) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    if (node->type != NODE_VARIABLE_DECLARATION) {
        return CODEGEN_ERROR_UNSUPPORTED_NODE;
    }

    // Allocate space on stack (8 bytes for int)
    generator->stack_offset += 8;
    fprintf(generator->output_file, "    sub     rsp, 8\n");

    // Generate initializer if present
    if (node->data.declaration.initializer) {
        CodeGenResult result = code_generator_generate_expression(generator, node->data.declaration.initializer);
        if (result != CODEGEN_SUCCESS) return result;

        // Store the value on stack
        fprintf(generator->output_file, "    mov     [rbp-%d], rax\n", generator->stack_offset);
    }

    return CODEGEN_SUCCESS;
}

CodeGenResult code_generator_generate_program(CodeGenerator* generator, ASTNode* node) {
    if (!generator || !node || !generator->output_file) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    CodeGenResult result = code_generator_emit_prologue(generator);
    if (result != CODEGEN_SUCCESS) return result;

    if (node->type == NODE_PROGRAM) {
        // Generate all children if it's a program node
        ASTNode* child = node->first_child;
        while (child) {
            switch (child->type) {
                case NODE_VARIABLE_DECLARATION:
                    result = code_generator_generate_variable_declaration(generator, child);
                    break;
                case NODE_BINARY_EXPRESSION:
                case NODE_LITERAL:
                case NODE_IDENTIFIER:
                    // Generate expression directly
                    result = code_generator_generate_expression(generator, child);
                    break;
                // TODO: Handle other statement types
                default:
                    result = CODEGEN_ERROR_UNSUPPORTED_NODE;
                    break;
            }

            if (result != CODEGEN_SUCCESS) return result;
            child = child->next_sibling;
        }
    } else {
        // Generate as single expression (for standalone expressions)
        result = code_generator_generate_expression(generator, node);
        if (result != CODEGEN_SUCCESS) return result;
    }

    result = code_generator_emit_epilogue(generator);
    return result;
}

CodeGenResult code_generator_generate(CodeGenerator* generator, ASTNode* ast, const char* output_filename) {
    if (!generator || !ast || !output_filename) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    CodeGenResult result = code_generator_set_output(generator, output_filename);
    if (result != CODEGEN_SUCCESS) return result;

    return code_generator_generate_program(generator, ast);
}

// Stub implementations for functions not yet implemented
CodeGenResult code_generator_generate_identifier(CodeGenerator* generator, ASTNode* node) {
    return CODEGEN_ERROR_UNSUPPORTED_NODE;
}

CodeGenResult code_generator_generate_unary(CodeGenerator* generator, ASTNode* node) {
    return CODEGEN_ERROR_UNSUPPORTED_NODE;
}

CodeGenResult code_generator_generate_assignment(CodeGenerator* generator, ASTNode* node) {
    return CODEGEN_ERROR_UNSUPPORTED_NODE;
}

CodeGenResult code_generator_emit_label(CodeGenerator* generator, const char* label) {
    if (!generator || !generator->output_file || !label) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    fprintf(generator->output_file, "%s:\n", label);
    return CODEGEN_SUCCESS;
}

CodeGenResult code_generator_push_stack(CodeGenerator* generator, int size) {
    if (!generator || !generator->output_file) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    generator->stack_offset += size;
    fprintf(generator->output_file, "    sub     rsp, %d\n", size);
    return CODEGEN_SUCCESS;
}

CodeGenResult code_generator_pop_stack(CodeGenerator* generator, int size) {
    if (!generator || !generator->output_file) {
        return CODEGEN_ERROR_NULL_ANALYZER;
    }

    generator->stack_offset -= size;
    fprintf(generator->output_file, "    add     rsp, %d\n", size);
    return CODEGEN_SUCCESS;
}