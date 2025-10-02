#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test framework macros
#define TEST_SUITE(name) \
    static void test_##name(void); \
    static void run_suite_##name(void) { \
        printf("Running test suite: %s\n", #name); \
        test_##name(); \
        printf("Test suite %s: PASSED\n\n", #name); \
    } \
    static void test_##name(void)

#define TEST_ASSERT(condition, message) \
    do { \
        test_count++; \
        if (!(condition)) { \
            printf("FAIL: %s at %s:%d\n", message, __FILE__, __LINE__); \
            failed_tests++; \
        } else { \
            passed_tests++; \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual, message) \
    do { \
        test_count++; \
        if ((expected) != (actual)) { \
            printf("FAIL: %s (expected: %d, actual: %d) at %s:%d\n", \
                   message, (int)(expected), (int)(actual), __FILE__, __LINE__); \
            failed_tests++; \
        } else { \
            passed_tests++; \
        } \
    } while(0)

#define TEST_ASSERT_STR_EQ(expected, actual, message) \
    do { \
        test_count++; \
        if (strcmp((expected), (actual)) != 0) { \
            printf("FAIL: %s (expected: '%s', actual: '%s') at %s:%d\n", \
                   message, (expected), (actual), __FILE__, __LINE__); \
            failed_tests++; \
        } else { \
            passed_tests++; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr, message) \
    do { \
        test_count++; \
        if ((ptr) != NULL) { \
            printf("FAIL: %s (expected NULL, got %p) at %s:%d\n", \
                   message, (ptr), __FILE__, __LINE__); \
            failed_tests++; \
        } else { \
            passed_tests++; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr, message) \
    do { \
        test_count++; \
        if ((ptr) == NULL) { \
            printf("FAIL: %s (expected non-NULL, got NULL) at %s:%d\n", \
                   message, __FILE__, __LINE__); \
            failed_tests++; \
        } else { \
            passed_tests++; \
        } \
    } while(0)

// Global test counters
extern int test_count;
extern int passed_tests;
extern int failed_tests;

// Test runner functions
void run_all_tests(void);
void print_test_results(void);
void reset_test_counters(void);

// Test suite declarations
void run_lexer_tests(void);
void run_parser_tests(void);
void run_semantic_analyzer_tests(void);
void run_code_generator_tests(void);
void run_integration_tests(void);

#endif // TEST_FRAMEWORK_H