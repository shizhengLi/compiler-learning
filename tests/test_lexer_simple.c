#include "lexer/lexer.h"
#include "lexer/token.h"
#include "common/common.h"

// Simple lexer test without the complex test framework
int main(void) {
    printf("=== Simple Lexer Test ===\n");

    // Test lexer creation
    Lexer* lexer = lexer_create("int x = 42;");
    if (lexer == NULL) {
        printf("FAIL: Lexer creation failed\n");
        return 1;
    }
    printf("PASS: Lexer created\n");

    // Test basic tokenization
    Token* token = lexer_next_token(lexer);
    if (token == NULL) {
        printf("FAIL: No token generated\n");
        lexer_free(lexer);
        return 1;
    }
    printf("PASS: Token generated: %s ('%s')\n", token_type_to_string(token->type), token->lexeme);

    token_free(token);
    lexer_free(lexer);

    printf("All simple tests passed!\n");
    return 0;
}