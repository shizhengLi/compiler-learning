#ifndef PARSER_H
#define PARSER_H

#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include "../common/common.h"

// Abstract Syntax Tree node types
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECLARATION,
    NODE_VARIABLE_DECLARATION,
    NODE_PARAMETER_LIST,
    NODE_BLOCK_STATEMENT,
    NODE_EXPRESSION_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_ASSIGNMENT_EXPRESSION,
    NODE_BINARY_EXPRESSION,
    NODE_UNARY_EXPRESSION,
    NODE_CALL_EXPRESSION,
    NODE_IDENTIFIER,
    NODE_LITERAL,
    NODE_ERROR
} NodeType;

// AST Node structure
typedef struct ASTNode {
    NodeType type;
    Token* token;
    struct ASTNode* parent;
    struct ASTNode* first_child;
    struct ASTNode* last_child;
    struct ASTNode* next_sibling;
    struct ASTNode* prev_sibling;
    union {
        // For expressions
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            char* operator;
        } binary;

        // For unary expressions
        struct {
            struct ASTNode* operand;
            char* operator;
        } unary;

        // For function calls
        struct {
            struct ASTNode* callee;
            struct ASTNode** arguments;
            int argument_count;
        } call;

        // For declarations
        struct {
            char* name;
            struct ASTNode* initializer;
            char* type_name;
            bool is_mutable;
        } declaration;

        // For statements
        struct {
            struct ASTNode** statements;
            int statement_count;
        } block;

        // For conditionals
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } conditional;

        // For literals
        union {
            int int_value;
            float float_value;
            char* string_value;
            char char_value;
            bool bool_value;
        } literal;

        // For identifiers
        char* identifier_name;

    } data;
    int line;
    int column;
} ASTNode;

// Parser structure
typedef struct Parser {
    Lexer* lexer;
    Token* current_token;
    Token* peek_token;
    bool had_error;
    Error* last_error;
} Parser;

// Parser creation and destruction
Parser* parser_create(Lexer* lexer);
void parser_free(Parser* parser);

// Main parsing functions
ASTNode* parser_parse(Parser* parser);
ASTNode* parser_parse_program(Parser* parser);

// Error handling
Error* parser_get_last_error(Parser* parser);
bool parser_had_error(Parser* parser);
void parser_clear_error(Parser* parser);

// AST Node functions
ASTNode* ast_node_create(NodeType type, Token* token);
void ast_node_free(ASTNode* node);
ASTNode* ast_node_create_binary(Token* token, ASTNode* left, ASTNode* right, const char* operator);
ASTNode* ast_node_create_unary(Token* token, ASTNode* operand, const char* operator);
ASTNode* ast_node_create_literal_int(Token* token, int value);
ASTNode* ast_node_create_literal_float(Token* token, float value);
ASTNode* ast_node_create_literal_string(Token* token, const char* value);
ASTNode* ast_node_create_identifier(Token* token, const char* name);
ASTNode* ast_node_create_variable_declaration(Token* token, const char* type_name, const char* var_name, ASTNode* initializer);
ASTNode* ast_node_create_program(void);
void ast_node_add_child(ASTNode* parent, ASTNode* child);

// Debug functions
const char* node_type_to_string(NodeType type);
void ast_node_print(ASTNode* node, int depth);

#endif // PARSER_H