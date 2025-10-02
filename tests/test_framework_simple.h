#ifndef TEST_FRAMEWORK_SIMPLE_H
#define TEST_FRAMEWORK_SIMPLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global test counters
extern int test_count;
extern int passed_tests;
extern int failed_tests;

// Test macros
#define TEST_ASSERT(condition, message) do { \
    test_count++; \
    if (!(condition)) { \
        printf("FAIL: %s at %s:%d\n", message, __FILE__, __LINE__); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

#define TEST_ASSERT_EQ(expected, actual, message) do { \
    test_count++; \
    if ((expected) != (actual)) { \
        printf("FAIL: %s (expected: %d, actual: %d) at %s:%d\n", message, (int)(expected), (int)(actual), __FILE__, __LINE__); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

#define TEST_ASSERT_STR_EQ(expected, actual, message) do { \
    test_count++; \
    if (strcmp(expected, actual) != 0) { \
        printf("FAIL: %s (expected: '%s', actual: '%s') at %s:%d\n", message, expected, actual, __FILE__, __LINE__); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

#define TEST_ASSERT_NOT_NULL(ptr, message) do { \
    test_count++; \
    if ((ptr) == NULL) { \
        printf("FAIL: %s (pointer is NULL) at %s:%d\n", message, __FILE__, __LINE__); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

#define TEST_ASSERT_NULL(ptr, message) do { \
    test_count++; \
    if ((ptr) != NULL) { \
        printf("FAIL: %s (pointer is not NULL) at %s:%d\n", message, __FILE__, __LINE__); \
        failed_tests++; \
    } else { \
        passed_tests++; \
    } \
} while(0)

// Test suite macros
#define TEST_SUITE(name) void run_suite_##name(void)

// Utility functions
void reset_test_counters(void);
void print_test_results(void);

#endif // TEST_FRAMEWORK_SIMPLE_H