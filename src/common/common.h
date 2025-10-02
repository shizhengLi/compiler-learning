#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

// Common macros
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Memory allocation with error checking
#define SAFE_MALLOC(ptr, size) \
    do { \
        ptr = malloc(size); \
        if (ptr == NULL) { \
            fprintf(stderr, "Memory allocation failed: %zu bytes at %s:%d\n", \
                    (size_t)(size), __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

#define SAFE_CALLOC(ptr, count, size) \
    do { \
        ptr = calloc(count, size); \
        if (ptr == NULL) { \
            fprintf(stderr, "Memory allocation failed: %zu * %zu bytes at %s:%d\n", \
                    (size_t)(count), (size_t)(size), __FILE__, __LINE__); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

#define SAFE_REALLOC(ptr, new_size) \
    do { \
        void* temp = realloc(ptr, new_size); \
        if (temp == NULL && new_size > 0) { \
            fprintf(stderr, "Memory reallocation failed: %zu bytes at %s:%d\n", \
                    (size_t)(new_size), __FILE__, __LINE__); \
            free(ptr); \
            exit(EXIT_FAILURE); \
        } \
        ptr = temp; \
    } while(0)

#define SAFE_FREE(ptr) \
    do { \
        if (ptr != NULL) { \
            free(ptr); \
            ptr = NULL; \
        } \
    } while(0)

// String utilities
char* strdup_safe(const char* str);
char* strndup_safe(const char* str, size_t n);

// Dynamic string buffer
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} StringBuffer;

StringBuffer* string_buffer_create(size_t initial_capacity);
void string_buffer_append(StringBuffer* buffer, const char* str);
void string_buffer_append_char(StringBuffer* buffer, char c);
void string_buffer_free(StringBuffer* buffer);

// Error handling
typedef enum {
    ERROR_NONE = 0,
    ERROR_LEXICAL,
    ERROR_SYNTAX,
    ERROR_SEMANTIC,
    ERROR_CODE_GENERATION,
    ERROR_MEMORY,
    ERROR_IO
} ErrorCode;

typedef struct {
    ErrorCode code;
    char* message;
    int line;
    int column;
    const char* file;
} Error;

Error* error_create(ErrorCode code, const char* message, int line, int column, const char* file);
void error_free(Error* error);
void error_print(Error* error);

#endif // COMMON_H