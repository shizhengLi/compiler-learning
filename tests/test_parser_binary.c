#include "../src/parser/parser.h"
#include <stdio.h>
#include <string.h>

void test_binary_expression(const char* input, const char* op, int left_val, int right_val) {
    printf("Testing binary: '%s'\n", input);

    Lexer* lexer = lexer_create(input);
    if (lexer == NULL) {
        printf("  FAIL: Lexer creation failed\n");
        return;
    }

    Parser* parser = parser_create(lexer);
    if (parser == NULL) {
        printf("  FAIL: Parser creation failed\n");
        lexer_free(lexer);
        return;
    }

    ASTNode* node = parser_parse(parser);
    if (node == NULL) {
        printf("  FAIL: Parser returned NULL\n");
        parser_free(parser);
        lexer_free(lexer);
        return;
    }

    if (node->type == NODE_BINARY_EXPRESSION) {
        printf("  PASS: Got BINARY_EXPRESSION\n");
        if (node->data.binary.left && node->data.binary.right &&
            node->data.binary.operator && strcmp(node->data.binary.operator, op) == 0) {
            printf("  PASS: Operator is '%s'\n", op);
            if (node->data.binary.left->type == NODE_LITERAL &&
                node->data.binary.right->type == NODE_LITERAL) {
                if (node->data.binary.left->data.literal.int_value == left_val &&
                    node->data.binary.right->data.literal.int_value == right_val) {
                    printf("  PASS: Values %d %s %d\n", left_val, op, right_val);
                } else {
                    printf("  FAIL: Wrong values\n");
                }
            } else {
                printf("  FAIL: Children are not literals\n");
            }
        } else {
            printf("  FAIL: Wrong operator or missing children\n");
        }
    } else {
        printf("  FAIL: Expected BINARY_EXPRESSION, got %s\n", node_type_to_string(node->type));
    }

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

int main(void) {
    printf("=== Parser Binary Expression Tests ===\n\n");

    // These tests will fail initially but guide implementation
    printf("Binary expression tests (expected to fail initially):\n");
    test_binary_expression("1 + 2", "+", 1, 2);
    test_binary_expression("3 * 4", "*", 3, 4);
    test_binary_expression("5 - 6", "-", 5, 6);

    printf("\nAll binary expression tests completed!\n");
    return 0;
}