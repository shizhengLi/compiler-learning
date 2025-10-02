#include "semantic.h"

// Symbol table functions
SymbolTable* symbol_table_create(int scope_level) {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    if (table == NULL) return NULL;

    table->symbols = malloc(sizeof(Symbol*) * 16);
    if (table->symbols == NULL) {
        free(table);
        return NULL;
    }

    table->symbol_count = 0;
    table->capacity = 16;
    table->parent = NULL;
    table->scope_level = scope_level;

    return table;
}

void symbol_table_free(SymbolTable* table) {
    if (table == NULL) return;

    for (int i = 0; i < table->symbol_count; i++) {
        symbol_free(table->symbols[i]);
    }

    free(table->symbols);
    free(table);
}

Symbol* symbol_table_add(SymbolTable* table, Symbol* symbol) {
    if (table == NULL || symbol == NULL) return NULL;

    // Check if we need to expand capacity
    if (table->symbol_count >= table->capacity) {
        int new_capacity = table->capacity * 2;
        Symbol** new_symbols = realloc(table->symbols, sizeof(Symbol*) * new_capacity);
        if (new_symbols == NULL) return NULL;

        table->symbols = new_symbols;
        table->capacity = new_capacity;
    }

    table->symbols[table->symbol_count++] = symbol;
    return symbol;
}

Symbol* symbol_table_lookup(SymbolTable* table, const char* name) {
    if (table == NULL || name == NULL) return NULL;

    // First check current scope
    Symbol* found = symbol_table_lookup_local(table, name);
    if (found != NULL) return found;

    // Then check parent scopes
    return symbol_table_lookup_global(table, name);
}

Symbol* symbol_table_lookup_local(SymbolTable* table, const char* name) {
    if (table == NULL || name == NULL) return NULL;

    for (int i = 0; i < table->symbol_count; i++) {
        if (strcmp(table->symbols[i]->name, name) == 0) {
            return table->symbols[i];
        }
    }

    return NULL;
}

Symbol* symbol_table_lookup_global(SymbolTable* table, const char* name) {
    if (table == NULL || name == NULL) return NULL;

    if (table->parent == NULL) return NULL;
    return symbol_table_lookup(table->parent, name);
}

// Symbol functions
Symbol* symbol_create_variable(const char* name, const char* type_name, bool is_mutable, int line, int column) {
    Symbol* symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) return NULL;

    symbol->name = strdup_safe(name);
    symbol->type = SYMBOL_VARIABLE;
    symbol->data.variable.type_name = strdup_safe(type_name);
    symbol->data.variable.is_mutable = is_mutable;
    symbol->scope_level = 0; // Will be set when added to scope
    symbol->line = line;
    symbol->column = column;

    return symbol;
}

Symbol* symbol_create_function(const char* name, const char* return_type, int line, int column) {
    Symbol* symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) return NULL;

    symbol->name = strdup_safe(name);
    symbol->type = SYMBOL_FUNCTION;
    symbol->data.function.return_type = strdup_safe(return_type);
    symbol->data.function.parameters = NULL;
    symbol->data.function.parameter_count = 0;
    symbol->scope_level = 0;
    symbol->line = line;
    symbol->column = column;

    return symbol;
}

Symbol* symbol_create_parameter(const char* name, const char* type_name, int position, int line, int column) {
    Symbol* symbol = malloc(sizeof(Symbol));
    if (symbol == NULL) return NULL;

    symbol->name = strdup_safe(name);
    symbol->type = SYMBOL_PARAMETER;
    symbol->data.parameter.type_name = strdup_safe(type_name);
    symbol->data.parameter.position = position;
    symbol->scope_level = 0;
    symbol->line = line;
    symbol->column = column;

    return symbol;
}

void symbol_free(Symbol* symbol) {
    if (symbol == NULL) return;

    free(symbol->name);

    switch (symbol->type) {
        case SYMBOL_VARIABLE:
            free(symbol->data.variable.type_name);
            break;
        case SYMBOL_FUNCTION:
            free(symbol->data.function.return_type);
            if (symbol->data.function.parameters) {
                for (int i = 0; i < symbol->data.function.parameter_count; i++) {
                    symbol_free(symbol->data.function.parameters[i]);
                }
                free(symbol->data.function.parameters);
            }
            break;
        case SYMBOL_PARAMETER:
            free(symbol->data.parameter.type_name);
            break;
        default:
            break;
    }

    free(symbol);
}

// Semantic analyzer functions
SemanticAnalyzer* semantic_analyzer_create(void) {
    SemanticAnalyzer* analyzer = malloc(sizeof(SemanticAnalyzer));
    if (analyzer == NULL) return NULL;

    analyzer->current_scope = symbol_table_create(0);
    analyzer->scope_stack = malloc(sizeof(SymbolTable*) * 16);
    analyzer->scope_stack[0] = analyzer->current_scope;
    analyzer->scope_stack_size = 1;
    analyzer->scope_stack_capacity = 16;
    analyzer->had_error = false;
    analyzer->last_error = NULL;

    return analyzer;
}

void semantic_analyzer_free(SemanticAnalyzer* analyzer) {
    if (analyzer == NULL) return;

    // Free all scopes in the stack
    for (int i = 0; i < analyzer->scope_stack_size; i++) {
        symbol_table_free(analyzer->scope_stack[i]);
    }

    free(analyzer->scope_stack);
    error_free(analyzer->last_error);
    free(analyzer);
}

bool semantic_analyze(ASTNode* node, SemanticAnalyzer* analyzer) {
    if (node == NULL || analyzer == NULL) {
        return false;
    }

    // For now, just get the type of the node
    DataType type = ast_node_get_type(node, analyzer);
    return type != TYPE_ERROR;
}

// Scope management
void semantic_analyzer_enter_scope(SemanticAnalyzer* analyzer) {
    if (analyzer == NULL) return;

    SymbolTable* new_scope = symbol_table_create(analyzer->current_scope->scope_level + 1);
    new_scope->parent = analyzer->current_scope;

    // Add to scope stack
    if (analyzer->scope_stack_size >= analyzer->scope_stack_capacity) {
        analyzer->scope_stack_capacity *= 2;
        analyzer->scope_stack = realloc(analyzer->scope_stack,
                                       sizeof(SymbolTable*) * analyzer->scope_stack_capacity);
    }

    analyzer->scope_stack[analyzer->scope_stack_size++] = new_scope;
    analyzer->current_scope = new_scope;
}

void semantic_analyzer_exit_scope(SemanticAnalyzer* analyzer) {
    if (analyzer == NULL || analyzer->scope_stack_size <= 1) return;

    // Don't free the global scope
    if (analyzer->scope_stack_size > 1) {
        analyzer->scope_stack_size--;
        symbol_table_free(analyzer->current_scope);
    }

    analyzer->current_scope = analyzer->scope_stack[analyzer->scope_stack_size - 1];
}

SymbolTable* semantic_analyzer_current_scope(SemanticAnalyzer* analyzer) {
    return analyzer ? analyzer->current_scope : NULL;
}

// Error handling
Error* semantic_analyzer_get_last_error(SemanticAnalyzer* analyzer) {
    return analyzer ? analyzer->last_error : NULL;
}

bool semantic_analyzer_had_error(SemanticAnalyzer* analyzer) {
    return analyzer ? analyzer->had_error : false;
}

void semantic_analyzer_clear_error(SemanticAnalyzer* analyzer) {
    if (analyzer) {
        analyzer->had_error = false;
        error_free(analyzer->last_error);
        analyzer->last_error = NULL;
    }
}

// Type checking functions
DataType ast_node_get_type(ASTNode* node, SemanticAnalyzer* analyzer) {
    if (node == NULL) return TYPE_ERROR;

    switch (node->type) {
        case NODE_LITERAL:
            if (node->token && node->token->type == TOKEN_INTEGER_LITERAL) {
                return TYPE_INT;
            } else if (node->token && node->token->type == TOKEN_FLOAT_LITERAL) {
                return TYPE_FLOAT;
            } else if (node->token && node->token->type == TOKEN_STRING_LITERAL) {
                return TYPE_STRING;
            } else if (node->token && (node->token->type == TOKEN_TRUE || node->token->type == TOKEN_FALSE)) {
                return TYPE_BOOL;
            }
            return TYPE_UNKNOWN;

        case NODE_IDENTIFIER:
            // Look up identifier in symbol table
            if (analyzer && node->data.identifier_name) {
                Symbol* symbol = symbol_table_lookup(analyzer->current_scope, node->data.identifier_name);
                if (symbol && symbol->type == SYMBOL_VARIABLE) {
                    // Map string type names to DataType enum
                    if (strcmp(symbol->data.variable.type_name, "int") == 0) return TYPE_INT;
                    if (strcmp(symbol->data.variable.type_name, "float") == 0) return TYPE_FLOAT;
                    if (strcmp(symbol->data.variable.type_name, "string") == 0) return TYPE_STRING;
                    if (strcmp(symbol->data.variable.type_name, "bool") == 0) return TYPE_BOOL;
                    if (strcmp(symbol->data.variable.type_name, "void") == 0) return TYPE_VOID;
                }
            }
            return TYPE_UNKNOWN;

        case NODE_BINARY_EXPRESSION:
            {
                DataType left_type = ast_node_get_type(node->data.binary.left, analyzer);
                DataType right_type = ast_node_get_type(node->data.binary.right, analyzer);

                // For arithmetic operations, if both operands are the same type, return that type
                if (left_type == right_type && (left_type == TYPE_INT || left_type == TYPE_FLOAT)) {
                    return left_type;
                }

                // For comparison operations, always return bool
                if (node->data.binary.operator &&
                    (strcmp(node->data.binary.operator, "==") == 0 ||
                     strcmp(node->data.binary.operator, "!=") == 0 ||
                     strcmp(node->data.binary.operator, "<") == 0 ||
                     strcmp(node->data.binary.operator, "<=") == 0 ||
                     strcmp(node->data.binary.operator, ">") == 0 ||
                     strcmp(node->data.binary.operator, ">=") == 0)) {
                    return TYPE_BOOL;
                }

                // For logical operations, return bool
                if (node->data.binary.operator &&
                    (strcmp(node->data.binary.operator, "&&") == 0 ||
                     strcmp(node->data.binary.operator, "||") == 0)) {
                    return TYPE_BOOL;
                }

                return TYPE_ERROR;
            }

        default:
            return TYPE_UNKNOWN;
    }
}

bool semantic_check_assignment(ASTNode* target, ASTNode* value, SemanticAnalyzer* analyzer) {
    if (target == NULL || value == NULL || analyzer == NULL) return false;

    DataType target_type = ast_node_get_type(target, analyzer);
    DataType value_type = ast_node_get_type(value, analyzer);

    // For now, allow assignment if types are the same
    return target_type == value_type && target_type != TYPE_ERROR;
}

bool semantic_check_binary_operation(ASTNode* left, ASTNode* right, const char* op, SemanticAnalyzer* analyzer) {
    if (left == NULL || right == NULL || op == NULL || analyzer == NULL) return false;

    DataType left_type = ast_node_get_type(left, analyzer);
    DataType right_type = ast_node_get_type(right, analyzer);

    // Allow arithmetic operations on same numeric types
    if ((left_type == TYPE_INT || left_type == TYPE_FLOAT) && left_type == right_type) {
        return true;
    }

    // Allow comparison operations on any types
    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
        strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 ||
        strcmp(op, ">") == 0 || strcmp(op, ">=") == 0) {
        return true;
    }

    // Allow logical operations on boolean types
    if ((strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) &&
        left_type == TYPE_BOOL && right_type == TYPE_BOOL) {
        return true;
    }

    return false;
}

// Utility functions
const char* data_type_to_string(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_CHAR: return "char";
        case TYPE_BOOL: return "bool";
        case TYPE_VOID: return "void";
        case TYPE_UNKNOWN: return "unknown";
        case TYPE_ERROR: return "error";
        default: return "invalid";
    }
}

const char* symbol_type_to_string(SymbolType type) {
    switch (type) {
        case SYMBOL_VARIABLE: return "VARIABLE";
        case SYMBOL_FUNCTION: return "FUNCTION";
        case SYMBOL_PARAMETER: return "PARAMETER";
        case SYMBOL_TYPE: return "TYPE";
        default: return "INVALID";
    }
}