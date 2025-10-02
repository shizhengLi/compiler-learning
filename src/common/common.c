#include "common.h"

// String utilities
char* strdup_safe(const char* str) {
    if (str == NULL) return NULL;

    size_t len = strlen(str);
    char* result = malloc(len + 1);
    if (result == NULL) {
        return NULL;
    }

    strcpy(result, str);
    return result;
}

char* strndup_safe(const char* str, size_t n) {
    if (str == NULL) return NULL;

    size_t len = strlen(str);
    if (len > n) len = n;

    char* result = malloc(len + 1);
    if (result == NULL) {
        return NULL;
    }

    strncpy(result, str, len);
    result[len] = '\0';
    return result;
}

// Dynamic string buffer
StringBuffer* string_buffer_create(size_t initial_capacity) {
    StringBuffer* buffer = malloc(sizeof(StringBuffer));
    if (buffer == NULL) return NULL;

    buffer->data = malloc(initial_capacity);
    if (buffer->data == NULL) {
        free(buffer);
        return NULL;
    }

    buffer->data[0] = '\0';
    buffer->length = 0;
    buffer->capacity = initial_capacity;

    return buffer;
}

void string_buffer_append(StringBuffer* buffer, const char* str) {
    if (buffer == NULL || str == NULL) return;

    size_t str_len = strlen(str);
    size_t new_length = buffer->length + str_len;

    if (new_length + 1 > buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2;
        if (new_capacity < new_length + 1) {
            new_capacity = new_length + 1;
        }

        char* new_data = realloc(buffer->data, new_capacity);
        if (new_data == NULL) return;

        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }

    strcpy(buffer->data + buffer->length, str);
    buffer->length = new_length;
}

void string_buffer_append_char(StringBuffer* buffer, char c) {
    if (buffer == NULL) return;

    if (buffer->length + 1 >= buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2;
        if (new_capacity == 0) new_capacity = 16;

        char* new_data = realloc(buffer->data, new_capacity);
        if (new_data == NULL) return;

        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }

    buffer->data[buffer->length] = c;
    buffer->data[buffer->length + 1] = '\0';
    buffer->length++;
}

void string_buffer_free(StringBuffer* buffer) {
    if (buffer == NULL) return;

    free(buffer->data);
    free(buffer);
}

// Error handling
Error* error_create(ErrorCode code, const char* message, int line, int column, const char* file) {
    Error* error = malloc(sizeof(Error));
    if (error == NULL) return NULL;

    error->code = code;
    error->message = strdup_safe(message);
    error->line = line;
    error->column = column;
    error->file = file;

    return error;
}

void error_free(Error* error) {
    if (error == NULL) return;

    free(error->message);
    free(error);
}

void error_print(Error* error) {
    if (error == NULL) return;

    const char* error_names[] = {
        "None",
        "Lexical",
        "Syntax",
        "Semantic",
        "Code Generation",
        "Memory",
        "IO"
    };

    const char* error_name = (error->code >= 0 && error->code < sizeof(error_names)/sizeof(error_names[0]))
                           ? error_names[error->code] : "Unknown";

    fprintf(stderr, "Error [%s]: %s", error_name, error->message);
    if (error->line > 0) {
        fprintf(stderr, " at line %d", error->line);
        if (error->column > 0) {
            fprintf(stderr, ":%d", error->column);
        }
    }
    if (error->file != NULL) {
        fprintf(stderr, " in %s", error->file);
    }
    fprintf(stderr, "\n");
}