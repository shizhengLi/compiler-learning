#include "../src/parser/parser.h"
#include <stdio.h>
#include <assert.h>

void test_case(const char* input, const char* expected_type) {
    printf("Testing: '%s'\n", input);

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

    const char* actual_type = node_type_to_string(node->type);
    if (strcmp(expected_type, actual_type) == 0) {
        printf("  PASS: Got %s as expected\n", actual_type);
    } else {
        printf("  FAIL: Expected %s, got %s\n", expected_type, actual_type);
    }

    ast_node_free(node);
    parser_free(parser);
    lexer_free(lexer);
}

int main(void) {
    printf("=== Parser Multiple Cases Test ===\n\n");

    test_case("42", "LITERAL");
    test_case("123", "LITERAL");
    test_case("variable", "IDENTIFIER");
    test_case("x", "IDENTIFIER");
    test_case("my_variable", "IDENTIFIER");
    test_case("+", "ERROR");  // Should error on operators for now

    printf("\nAll tests completed!\n");
    return 0;
}