#include "parser.h"

// Forward declarations for static functions
static ASTNode* parser_parse_expression(Parser* parser);

Parser* parser_create(Lexer* lexer) {
    if (lexer == NULL) return NULL;

    Parser* parser = malloc(sizeof(Parser));
    if (parser == NULL) return NULL;

    parser->lexer = lexer;
    parser->current_token = lexer_next_token(lexer);
    parser->peek_token = lexer_next_token(lexer);
    parser->had_error = false;
    parser->last_error = NULL;

    return parser;
}

void parser_free(Parser* parser) {
    if (parser == NULL) return;

    token_free(parser->current_token);
    token_free(parser->peek_token);
    error_free(parser->last_error);
    free(parser);
}

ASTNode* parser_parse(Parser* parser) {
    if (parser == NULL) return NULL;

    // For now, just parse a simple expression
    return parser_parse_expression(parser);
}

ASTNode* parser_parse_program(Parser* parser) {
    if (parser == NULL) return NULL;

    ASTNode* program = ast_node_create(NODE_PROGRAM, NULL);
    if (program == NULL) return NULL;

    // TODO: Implement program parsing
    return program;
}

Error* parser_get_last_error(Parser* parser) {
    return parser ? parser->last_error : NULL;
}

bool parser_had_error(Parser* parser) {
    return parser ? parser->had_error : false;
}

void parser_clear_error(Parser* parser) {
    if (parser) {
        parser->had_error = false;
        error_free(parser->last_error);
        parser->last_error = NULL;
    }
}

// AST Node functions
ASTNode* ast_node_create(NodeType type, Token* token) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (node == NULL) return NULL;

    node->type = type;
    node->token = token;
    node->line = token ? token->line : 0;
    node->column = token ? token->column : 0;

    // Initialize all fields to NULL/0
    memset(&node->data, 0, sizeof(node->data));

    return node;
}

void ast_node_free(ASTNode* node) {
    if (node == NULL) return;

    // Free based on node type
    switch (node->type) {
        case NODE_BINARY_EXPRESSION:
            ast_node_free(node->data.binary.left);
            ast_node_free(node->data.binary.right);
            free(node->data.binary.operator);
            break;

        case NODE_UNARY_EXPRESSION:
            ast_node_free(node->data.unary.operand);
            free(node->data.unary.operator);
            break;

        case NODE_CALL_EXPRESSION:
            ast_node_free(node->data.call.callee);
            for (int i = 0; i < node->data.call.argument_count; i++) {
                ast_node_free(node->data.call.arguments[i]);
            }
            free(node->data.call.arguments);
            break;

        case NODE_VARIABLE_DECLARATION:
            free(node->data.declaration.name);
            free(node->data.declaration.type_name);
            ast_node_free(node->data.declaration.initializer);
            break;

        case NODE_BLOCK_STATEMENT:
            for (int i = 0; i < node->data.block.statement_count; i++) {
                ast_node_free(node->data.block.statements[i]);
            }
            free(node->data.block.statements);
            break;

        case NODE_IF_STATEMENT:
            ast_node_free(node->data.conditional.condition);
            ast_node_free(node->data.conditional.then_branch);
            ast_node_free(node->data.conditional.else_branch);
            break;

        case NODE_LITERAL:
            if (node->token && node->token->type == TOKEN_STRING_LITERAL) {
                free(node->data.literal.string_value);
            }
            break;

        case NODE_IDENTIFIER:
            free(node->data.identifier_name);
            break;

        default:
            break;
    }

    token_free(node->token);
    free(node);
}

ASTNode* ast_node_create_binary(Token* token, ASTNode* left, ASTNode* right, const char* operator) {
    ASTNode* node = ast_node_create(NODE_BINARY_EXPRESSION, token);
    if (node == NULL) return NULL;

    node->data.binary.left = left;
    node->data.binary.right = right;
    node->data.binary.operator = strdup_safe(operator);

    return node;
}

ASTNode* ast_node_create_unary(Token* token, ASTNode* operand, const char* operator) {
    ASTNode* node = ast_node_create(NODE_UNARY_EXPRESSION, token);
    if (node == NULL) return NULL;

    node->data.unary.operand = operand;
    node->data.unary.operator = strdup_safe(operator);

    return node;
}

ASTNode* ast_node_create_literal_int(Token* token, int value) {
    ASTNode* node = ast_node_create(NODE_LITERAL, token);
    if (node == NULL) return NULL;

    node->data.literal.int_value = value;
    return node;
}

ASTNode* ast_node_create_literal_float(Token* token, float value) {
    ASTNode* node = ast_node_create(NODE_LITERAL, token);
    if (node == NULL) return NULL;

    node->data.literal.float_value = value;
    return node;
}

ASTNode* ast_node_create_literal_string(Token* token, const char* value) {
    ASTNode* node = ast_node_create(NODE_LITERAL, token);
    if (node == NULL) return NULL;

    node->data.literal.string_value = strdup_safe(value);
    return node;
}

ASTNode* ast_node_create_identifier(Token* token, const char* name) {
    ASTNode* node = ast_node_create(NODE_IDENTIFIER, token);
    if (node == NULL) return NULL;

    node->data.identifier_name = strdup_safe(name);
    return node;
}

// Debug functions
const char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "PROGRAM";
        case NODE_FUNCTION_DECLARATION: return "FUNCTION_DECLARATION";
        case NODE_VARIABLE_DECLARATION: return "VARIABLE_DECLARATION";
        case NODE_PARAMETER_LIST: return "PARAMETER_LIST";
        case NODE_BLOCK_STATEMENT: return "BLOCK_STATEMENT";
        case NODE_EXPRESSION_STATEMENT: return "EXPRESSION_STATEMENT";
        case NODE_RETURN_STATEMENT: return "RETURN_STATEMENT";
        case NODE_IF_STATEMENT: return "IF_STATEMENT";
        case NODE_WHILE_STATEMENT: return "WHILE_STATEMENT";
        case NODE_ASSIGNMENT_EXPRESSION: return "ASSIGNMENT_EXPRESSION";
        case NODE_BINARY_EXPRESSION: return "BINARY_EXPRESSION";
        case NODE_UNARY_EXPRESSION: return "UNARY_EXPRESSION";
        case NODE_CALL_EXPRESSION: return "CALL_EXPRESSION";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_LITERAL: return "LITERAL";
        case NODE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void ast_node_print(ASTNode* node, int depth) {
    if (node == NULL) return;

    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    printf("%s", node_type_to_string(node->type));

    switch (node->type) {
        case NODE_LITERAL:
            if (node->token && node->token->type == TOKEN_INTEGER_LITERAL) {
                printf(" (%d)", node->data.literal.int_value);
            } else if (node->token && node->token->type == TOKEN_STRING_LITERAL) {
                printf(" (\"%s\")", node->data.literal.string_value);
            }
            break;
        case NODE_IDENTIFIER:
            printf(" (%s)", node->data.identifier_name);
            break;
        case NODE_BINARY_EXPRESSION:
            printf(" (%s)", node->data.binary.operator);
            break;
        case NODE_UNARY_EXPRESSION:
            printf(" (%s)", node->data.unary.operator);
            break;
        default:
            break;
    }

    printf("\n");

    // Print children based on node type
    switch (node->type) {
        case NODE_BINARY_EXPRESSION:
            ast_node_print(node->data.binary.left, depth + 1);
            ast_node_print(node->data.binary.right, depth + 1);
            break;

        case NODE_UNARY_EXPRESSION:
            ast_node_print(node->data.unary.operand, depth + 1);
            break;

        case NODE_CALL_EXPRESSION:
            ast_node_print(node->data.call.callee, depth + 1);
            for (int i = 0; i < node->data.call.argument_count; i++) {
                ast_node_print(node->data.call.arguments[i], depth + 1);
            }
            break;

        default:
            break;
    }
}

// Parser helper functions (static)
static ASTNode* parser_parse_expression(Parser* parser) {
    if (parser == NULL || parser->current_token == NULL) {
        return ast_node_create(NODE_ERROR, NULL);
    }

    Token* token = parser->current_token;

    // Handle EOF token
    if (token->type == TOKEN_EOF) {
        return ast_node_create(NODE_ERROR, token);
    }

    switch (token->type) {
        case TOKEN_INTEGER_LITERAL:
            {
                ASTNode* node = ast_node_create_literal_int(token, token->literal.int_value);
                // Advance tokens safely
                Token* next = parser->peek_token;
                if (next != NULL) {
                    parser->current_token = next;
                    parser->peek_token = lexer_next_token(parser->lexer);
                } else {
                    parser->current_token = NULL;
                    parser->peek_token = NULL;
                }
                return node;
            }

        case TOKEN_FLOAT_LITERAL:
            {
                ASTNode* node = ast_node_create_literal_float(token, token->literal.float_value);
                // Advance tokens safely
                Token* next = parser->peek_token;
                if (next != NULL) {
                    parser->current_token = next;
                    parser->peek_token = lexer_next_token(parser->lexer);
                } else {
                    parser->current_token = NULL;
                    parser->peek_token = NULL;
                }
                return node;
            }

        case TOKEN_STRING_LITERAL:
            {
                ASTNode* node = ast_node_create_literal_string(token, token->literal.string_value);
                // Advance tokens safely
                Token* next = parser->peek_token;
                if (next != NULL) {
                    parser->current_token = next;
                    parser->peek_token = lexer_next_token(parser->lexer);
                } else {
                    parser->current_token = NULL;
                    parser->peek_token = NULL;
                }
                return node;
            }

        case TOKEN_IDENTIFIER:
            {
                ASTNode* node = ast_node_create_identifier(token, token->lexeme);
                // Advance tokens safely
                Token* next = parser->peek_token;
                if (next != NULL) {
                    parser->current_token = next;
                    parser->peek_token = lexer_next_token(parser->lexer);
                } else {
                    parser->current_token = NULL;
                    parser->peek_token = NULL;
                }
                return node;
            }

        default:
            parser->had_error = true;
            parser->last_error = error_create(ERROR_SYNTAX, "Unexpected token in expression",
                                             token->line, token->column, token->lexeme);
            return ast_node_create(NODE_ERROR, token);
    }
}