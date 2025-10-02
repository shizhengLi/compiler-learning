#include "../src/parser/parser.h"
#include <stdio.h>
#include <string.h>

void print_ast(ASTNode* node, int depth) {
    if (node == NULL) return;

    for (int i = 0; i < depth; i++) printf("  ");
    printf("%s", node_type_to_string(node->type));

    if (node->type == NODE_BINARY_EXPRESSION) {
        printf(" (%s)", node->data.binary.operator);
    } else if (node->type == NODE_LITERAL) {
        printf(" (%d)", node->data.literal.int_value);
    } else if (node->type == NODE_IDENTIFIER) {
        printf(" (%s)", node->data.identifier_name);
    }
    printf("\n");

    if (node->type == NODE_BINARY_EXPRESSION) {
        print_ast(node->data.binary.left, depth + 1);
        print_ast(node->data.binary.right, depth + 1);
    }
}

void test_precedence(const char* input, const char* description) {
    printf("Testing precedence: '%s' (%s)\n", input, description);

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

    print_ast(node, 1);
    printf("\n");

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

int main(void) {
    printf("=== Parser Operator Precedence Tests ===\n\n");

    // Test basic operator precedence (should be left-associative for now)
    test_precedence("1 + 2 * 3", "multiplication should bind tighter");
    test_precedence("1 * 2 + 3", "addition and multiplication");
    test_precedence("1 + 2 + 3", "left-associative addition");
    test_precedence("1 * 2 * 3", "left-associative multiplication");
    test_precedence("10 - 5 - 2", "left-associative subtraction");
    test_precedence("10 / 2 / 5", "left-associative division");

    printf("All precedence tests completed!\n");
    return 0;
}