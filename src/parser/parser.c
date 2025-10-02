#include "parser.h"
#include <stdbool.h>

// Minimal parser implementation for testing

// Forward declarations
static ASTNode* parser_parse_expression(Parser* parser);
static ASTNode* parser_parse_expression_precedence(Parser* parser, int precedence);
static ASTNode* parser_parse_primary(Parser* parser);
static bool parser_is_binary_operator(TokenType type);
static int parser_get_precedence(TokenType type);

Parser* parser_create(Lexer* lexer) {
    if (lexer == NULL) return NULL;

    Parser* parser = malloc(sizeof(Parser));
    if (parser == NULL) return NULL;

    parser->lexer = lexer;
    parser->current_token = NULL;
    parser->peek_token = NULL;
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

    // Get first token from lexer
    if (parser->current_token == NULL) {
        parser->current_token = lexer_next_token(parser->lexer);
    }

    if (parser->current_token == NULL || parser->current_token->type == TOKEN_EOF) {
        return ast_node_create(NODE_ERROR, NULL);
    }

    return parser_parse_expression(parser);
}

ASTNode* parser_parse_program(Parser* parser) {
    if (parser == NULL) return NULL;

    ASTNode* program = ast_node_create(NODE_PROGRAM, NULL);
    if (program == NULL) return NULL;

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

    // Initialize pointer fields
    node->parent = NULL;
    node->first_child = NULL;
    node->last_child = NULL;
    node->next_sibling = NULL;
    node->prev_sibling = NULL;

    // Initialize all fields in union to NULL/0
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
        case NODE_LITERAL: return "LITERAL";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_BINARY_EXPRESSION: return "BINARY_EXPRESSION";
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
            }
            break;
        case NODE_IDENTIFIER:
            printf(" (%s)", node->data.identifier_name);
            break;
        default:
            break;
    }

    printf("\n");
}

// Expression parsing with proper operator precedence
static ASTNode* parser_parse_expression(Parser* parser) {
    if (parser == NULL || parser->current_token == NULL) {
        return ast_node_create(NODE_ERROR, NULL);
    }

    // Parse at the lowest precedence level (addition/subtraction)
    return parser_parse_expression_precedence(parser, 1);
}

// Parse expression at specific precedence level
static ASTNode* parser_parse_expression_precedence(Parser* parser, int precedence) {
    if (parser == NULL || parser->current_token == NULL) {
        return ast_node_create(NODE_ERROR, NULL);
    }

    ASTNode* left = parser_parse_primary(parser);
    if (left == NULL || left->type == NODE_ERROR) {
        return left;
    }

    while (true) {
        Token* token = parser->current_token;
        if (token == NULL || !parser_is_binary_operator(token->type)) {
            break;
        }

        int token_precedence = parser_get_precedence(token->type);
        if (token_precedence < precedence) {
            break;
        }

        Token* op_token = token;
        char* operator = strdup_safe(op_token->lexeme);

        // Consume the operator
        parser->current_token = lexer_next_token(parser->lexer);

        // Parse right side with higher precedence
        ASTNode* right = parser_parse_expression_precedence(parser, token_precedence + 1);
        if (right == NULL || right->type == NODE_ERROR) {
            free(operator);
            return right;
        }

        // Create binary expression node
        ASTNode* binary = ast_node_create_binary(op_token, left, right, operator);
        free(operator);

        left = binary;
    }

    return left;
}

// Get operator precedence (higher number = higher precedence)
static int parser_get_precedence(TokenType type) {
    switch (type) {
        // Assignment (lowest precedence)
        case TOKEN_ASSIGN:
            return 1;

        // Logical OR
        case TOKEN_LOGICAL_OR:
            return 2;

        // Logical AND
        case TOKEN_LOGICAL_AND:
            return 3;

        // Equality
        case TOKEN_EQUAL:
        case TOKEN_NOT_EQUAL:
            return 4;

        // Comparison
        case TOKEN_LESS:
        case TOKEN_LESS_EQUAL:
        case TOKEN_GREATER:
        case TOKEN_GREATER_EQUAL:
            return 5;

        // Addition/Subtraction
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 6;

        // Multiplication/Division/Modulo
        case TOKEN_MULTIPLY:
        case TOKEN_DIVIDE:
        case TOKEN_MODULO:
            return 7;

        // Bitwise shift
        case TOKEN_LEFT_SHIFT:
        case TOKEN_RIGHT_SHIFT:
            return 8;

        // Bitwise AND
        case TOKEN_BITWISE_AND:
            return 9;

        // Bitwise XOR
        case TOKEN_BITWISE_XOR:
            return 10;

        // Bitwise OR
        case TOKEN_BITWISE_OR:
            return 11;

        default:
            return 0; // Not an operator
    }
}

// Parse primary expressions (literals, identifiers, parenthesized expressions, etc.)
static ASTNode* parser_parse_primary(Parser* parser) {
    if (parser == NULL || parser->current_token == NULL) {
        return ast_node_create(NODE_ERROR, NULL);
    }

    Token* token = parser->current_token;

    // Handle parenthesized expressions
    if (token->type == TOKEN_LEFT_PAREN) {
        // Consume '('
        parser->current_token = lexer_next_token(parser->lexer);

        // Parse the expression inside parentheses
        ASTNode* expr = parser_parse_expression(parser);
        if (expr == NULL || expr->type == NODE_ERROR) {
            return expr;
        }

        // Expect and consume ')'
        Token* closing_paren = parser->current_token;
        if (closing_paren == NULL || closing_paren->type != TOKEN_RIGHT_PAREN) {
            parser->had_error = true;
            parser->last_error = error_create(ERROR_SYNTAX, "Expected closing parenthesis",
                                             closing_paren ? closing_paren->line : 0,
                                             closing_paren ? closing_paren->column : 0,
                                             closing_paren ? closing_paren->lexeme : NULL);
            return ast_node_create(NODE_ERROR, closing_paren);
        }

        // Consume ')'
        parser->current_token = lexer_next_token(parser->lexer);
        return expr;
    }

    switch (token->type) {
        case TOKEN_INTEGER_LITERAL:
            {
                ASTNode* node = ast_node_create_literal_int(token, token->literal.int_value);
                // Advance to next token
                parser->current_token = lexer_next_token(parser->lexer);
                return node;
            }

        case TOKEN_FLOAT_LITERAL:
            {
                ASTNode* node = ast_node_create_literal_float(token, token->literal.float_value);
                // Advance to next token
                parser->current_token = lexer_next_token(parser->lexer);
                return node;
            }

        case TOKEN_STRING_LITERAL:
            {
                ASTNode* node = ast_node_create_literal_string(token, token->literal.string_value);
                // Advance to next token
                parser->current_token = lexer_next_token(parser->lexer);
                return node;
            }

        case TOKEN_IDENTIFIER:
            {
                ASTNode* node = ast_node_create_identifier(token, token->lexeme);
                // Advance to next token
                parser->current_token = lexer_next_token(parser->lexer);
                return node;
            }

        case TOKEN_TRUE:
            {
                ASTNode* node = ast_node_create_literal_int(token, 1); // true = 1
                // Advance to next token
                parser->current_token = lexer_next_token(parser->lexer);
                return node;
            }

        case TOKEN_FALSE:
            {
                ASTNode* node = ast_node_create_literal_int(token, 0); // false = 0
                // Advance to next token
                parser->current_token = lexer_next_token(parser->lexer);
                return node;
            }

        default:
            parser->had_error = true;
            parser->last_error = error_create(ERROR_SYNTAX, "Unexpected token in expression",
                                             token->line, token->column, token->lexeme);
            return ast_node_create(NODE_ERROR, token);
    }
}


// Check if token type is a binary operator
static bool parser_is_binary_operator(TokenType type) {
    switch (type) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_MULTIPLY:
        case TOKEN_DIVIDE:
        case TOKEN_MODULO:
        case TOKEN_EQUAL:
        case TOKEN_NOT_EQUAL:
        case TOKEN_LESS:
        case TOKEN_LESS_EQUAL:
        case TOKEN_GREATER:
        case TOKEN_GREATER_EQUAL:
        case TOKEN_LOGICAL_AND:
        case TOKEN_LOGICAL_OR:
        case TOKEN_BITWISE_AND:
        case TOKEN_BITWISE_OR:
        case TOKEN_BITWISE_XOR:
        case TOKEN_LEFT_SHIFT:
        case TOKEN_RIGHT_SHIFT:
            return true;
        default:
            return false;
    }
}

ASTNode* ast_node_create_variable_declaration(Token* token, const char* type_name, const char* var_name, ASTNode* initializer) {
    ASTNode* node = ast_node_create(NODE_VARIABLE_DECLARATION, token);
    if (node == NULL) return NULL;

    node->data.declaration.type_name = strdup_safe(type_name);
    node->data.declaration.name = strdup_safe(var_name);
    node->data.declaration.initializer = initializer;
    node->data.declaration.is_mutable = true; // Default to mutable

    return node;
}

ASTNode* ast_node_create_program(void) {
    ASTNode* node = ast_node_create(NODE_PROGRAM, NULL);
    if (node == NULL) return NULL;

    return node;
}

void ast_node_add_child(ASTNode* parent, ASTNode* child) {
    if (!parent || !child) return;

    if (parent->first_child == NULL) {
        // First child
        parent->first_child = child;
        parent->last_child = child;
    } else {
        // Add to end of children list
        child->prev_sibling = parent->last_child;
        parent->last_child->next_sibling = child;
        parent->last_child = child;
    }

    child->parent = parent;
}