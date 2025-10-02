#include "../src/parser/parser.h"

int main(void) {
    printf("=== Simple Parser Test ===\n");

    // Test parser creation
    Lexer* lexer = lexer_create("42");
    if (lexer == NULL) {
        printf("FAIL: Lexer creation failed\n");
        return 1;
    }

    Parser* parser = parser_create(lexer);
    if (parser == NULL) {
        printf("FAIL: Parser creation failed\n");
        lexer_free(lexer);
        return 1;
    }
    printf("PASS: Parser created\n");

    // Test parsing simple expression
    ASTNode* node = parser_parse(parser);
    if (node == NULL) {
        printf("FAIL: Parser returned NULL\n");
        parser_free(parser);
        lexer_free(lexer);
        return 1;
    }
    printf("PASS: Parser returned node: %s\n", node_type_to_string(node->type));

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);

    printf("All simple parser tests passed!\n");
    return 0;
}