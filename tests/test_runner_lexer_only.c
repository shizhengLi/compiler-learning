#include "test_framework.h"

// Include only lexer test suites
#include "unit/test_token.c"
#include "unit/test_lexer.c"

int main(void) {
    reset_test_counters();

    printf("=== LEXER TEST SUITE ===\n\n");

    // Run lexer tests only
    run_token_tests();
    run_lexer_tests();

    print_test_results();
    return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}