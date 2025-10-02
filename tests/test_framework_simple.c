#include "test_framework_simple.h"

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