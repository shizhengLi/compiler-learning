#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../parser/parser.h"
#include "../lexer/token.h"
#include "../common/common.h"

// Symbol types
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER,
    SYMBOL_TYPE
} SymbolType;

// Symbol structure
typedef struct Symbol {
    char* name;
    SymbolType type;
    union {
        struct {
            char* type_name;
            bool is_mutable;
        } variable;

        struct {
            char* return_type;
            struct Symbol** parameters;
            int parameter_count;
        } function;

        struct {
            char* type_name;
            int position;
        } parameter;

        struct {
            int size;
        } type_info;
    } data;
    int scope_level;
    int line;
    int column;
} Symbol;

// Symbol table structure
typedef struct SymbolTable {
    Symbol** symbols;
    int symbol_count;
    int capacity;
    struct SymbolTable* parent;  // For nested scopes
    int scope_level;
} SymbolTable;

// Semantic analyzer structure
typedef struct SemanticAnalyzer {
    SymbolTable* current_scope;
    bool had_error;
    Error* last_error;
    SymbolTable** scope_stack;
    int scope_stack_size;
    int scope_stack_capacity;
} SemanticAnalyzer;

// Type information
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_UNKNOWN,
    TYPE_ERROR
} DataType;

DataType ast_node_get_type(ASTNode* node, SemanticAnalyzer* analyzer);

// Symbol table functions
SymbolTable* symbol_table_create(int scope_level);
void symbol_table_free(SymbolTable* table);
Symbol* symbol_table_add(SymbolTable* table, Symbol* symbol);
Symbol* symbol_table_lookup(SymbolTable* table, const char* name);
Symbol* symbol_table_lookup_local(SymbolTable* table, const char* name);
Symbol* symbol_table_lookup_global(SymbolTable* table, const char* name);

// Symbol functions
Symbol* symbol_create_variable(const char* name, const char* type_name, bool is_mutable, int line, int column);
Symbol* symbol_create_function(const char* name, const char* return_type, int line, int column);
Symbol* symbol_create_parameter(const char* name, const char* type_name, int position, int line, int column);
void symbol_free(Symbol* symbol);

// Semantic analyzer functions
SemanticAnalyzer* semantic_analyzer_create(void);
void semantic_analyzer_free(SemanticAnalyzer* analyzer);
bool semantic_analyze(ASTNode* node, SemanticAnalyzer* analyzer);

// Scope management
void semantic_analyzer_enter_scope(SemanticAnalyzer* analyzer);
void semantic_analyzer_exit_scope(SemanticAnalyzer* analyzer);
SymbolTable* semantic_analyzer_current_scope(SemanticAnalyzer* analyzer);

// Error handling
Error* semantic_analyzer_get_last_error(SemanticAnalyzer* analyzer);
bool semantic_analyzer_had_error(SemanticAnalyzer* analyzer);
void semantic_analyzer_clear_error(SemanticAnalyzer* analyzer);

// Type checking functions
DataType semantic_check_expression(ASTNode* node, SemanticAnalyzer* analyzer);
bool semantic_check_assignment(ASTNode* target, ASTNode* value, SemanticAnalyzer* analyzer);
bool semantic_check_binary_operation(ASTNode* left, ASTNode* right, const char* op, SemanticAnalyzer* analyzer);

// Utility functions
const char* data_type_to_string(DataType type);
const char* symbol_type_to_string(SymbolType type);

#endif // SEMANTIC_H