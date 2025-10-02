#include "test_framework.h"

// Global test counters
int test_count = 0;
int passed_tests = 0;
int failed_tests = 0;

void reset_test_counters(void) {
    test_count = 0;
    passed_tests = 0;
    failed_tests = 0;
}

void print_test_results(void) {
    printf("\n=== TEST RESULTS ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success rate: %.1f%%\n",
           test_count > 0 ? (float)passed_tests / test_count * 100.0 : 0.0);

    if (failed_tests == 0) {
        printf("🎉 ALL TESTS PASSED! 🎉\n");
    } else {
        printf("❌ SOME TESTS FAILED ❌\n");
    }
}

// Empty implementations for test suites not yet implemented
void run_parser_tests(void) { printf("Parser tests not yet implemented\n"); }
void run_semantic_analyzer_tests(void) { printf("Semantic analyzer tests not yet implemented\n"); }
void run_code_generator_tests(void) { printf("Code generator tests not yet implemented\n"); }
void run_integration_tests(void) { printf("Integration tests not yet implemented\n"); }
void run_token_tests(void);

void run_all_tests(void) {
    reset_test_counters();

    printf("=== COMPILER TEST SUITE ===\n\n");

    // Run individual test suites
    run_token_tests();
    run_lexer_tests();
    run_parser_tests();
    run_semantic_analyzer_tests();
    run_code_generator_tests();
    run_integration_tests();

    print_test_results();
}