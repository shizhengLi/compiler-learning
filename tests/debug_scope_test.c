#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/semantic/semantic.h"
#include "src/common/common.h"

int main() {
    printf("=== DEBUG SCOPE TEST (REPLICATING ORIGINAL TEST) ===\n");

    // Create semantic analyzer
    SemanticAnalyzer* analyzer = semantic_analyzer_create();

    // Create symbol var1 just like in the original test
    Symbol* var1 = symbol_create_variable("count", "int", true, 5, 10);
    printf("Created var1: count at %p\n", (void*)var1);

    // Add var1 to global scope just like original test
    Symbol* added = symbol_table_add(analyzer->current_scope, var1);
    printf("Added var1 to global scope, returned: %p\n", (void*)added);

    // Test 5: Scope management (replicating original test exactly)
    printf("Test 5: Scope Management\n");

    // Add the global variable to the analyzer's symbol table for scope testing
    symbol_table_add(analyzer->current_scope, var1);
    printf("Added var1 to analyzer current scope\n");

    semantic_analyzer_enter_scope(analyzer);
    Symbol* local_var = symbol_create_variable("local", "bool", true, 10, 5);
    symbol_table_add(analyzer->current_scope, local_var);
    printf("Created and added local_var at %p\n", (void*)local_var);

    Symbol* found_local = symbol_table_lookup(analyzer->current_scope, "local");
    Symbol* found_global_in_local = symbol_table_lookup(analyzer->current_scope, "count");

    printf("found_local: %p (expected: %p) -> %s\n",
           (void*)found_local, (void*)local_var,
           found_local == local_var ? "MATCH" : "NO MATCH");
    printf("found_global_in_local: %p (expected: %p) -> %s\n",
           (void*)found_global_in_local, (void*)var1,
           found_global_in_local == var1 ? "MATCH" : "NO MATCH");

    semantic_analyzer_exit_scope(analyzer);
    Symbol* not_found_local = symbol_table_lookup(analyzer->current_scope, "local");

    if (analyzer->current_scope->scope_level == 0 &&
        found_local == local_var && found_global_in_local == var1 && not_found_local == NULL) {
        printf("  ✓ Scope management works correctly\n");
    } else {
        printf("  ✗ Scope management failed\n");
        printf("    Scope level: %d (expected 0)\n", analyzer->current_scope->scope_level);
        printf("    Local match: %s\n", found_local == local_var ? "YES" : "NO");
        printf("    Global match: %s\n", found_global_in_local == var1 ? "YES" : "NO");
        printf("    Local not found: %s\n", not_found_local == NULL ? "YES" : "NO");
    }

    semantic_analyzer_free(analyzer);
    return 0;
}