#include "test_framework.h"

// Include test suites
#include "unit/test_token.c"
#include "unit/test_lexer.c"
#include "unit/test_parser.c"
#include "unit/test_semantic.c"

int main(void) {
    run_all_tests();
    return (failed_tests == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}