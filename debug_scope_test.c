#include "src/semantic/semantic.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== DEBUGGING SCOPE MANAGEMENT TEST ===\n\n");

    // Initialize semantic analyzer
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("❌ Failed to create semantic analyzer\n");
        return 1;
    }
    printf("✓ Semantic analyzer created\n");
    printf("  Initial scope level: %d\n", analyzer->current_scope->scope_level);

    // Add global variable "count"
    Symbol* var1 = symbol_create_variable("count", "int", true, 5, 10);
    symbol_table_add(analyzer->current_scope, var1);
    printf("✓ Added global variable 'count'\n");

    // Enter local scope
    semantic_analyzer_enter_scope(analyzer);
    printf("✓ Entered local scope\n");
    printf("  Current scope level: %d\n", analyzer->current_scope->scope_level);

    // Add local variable "local"
    Symbol* local_var = symbol_create_variable("local", "bool", true, 10, 5);
    symbol_table_add(analyzer->current_scope, local_var);
    printf("✓ Added local variable 'local'\n");

    // Look up local variable
    Symbol* found_local = symbol_table_lookup(analyzer->current_scope, "local");
    printf("Found local variable: %s\n", found_local ? "YES" : "NO");
    if (found_local) {
        printf("  Name: %s, Type: %s\n", found_local->name, found_local->data.variable.type_name);
    }

    // Look up global variable from local scope
    Symbol* found_global_in_local = symbol_table_lookup(analyzer->current_scope, "count");
    printf("Found global variable 'count' from local scope: %s\n", found_global_in_local ? "YES" : "NO");
    if (found_global_in_local) {
        printf("  Name: %s, Type: %s\n", found_global_in_local->name, found_global_in_local->data.variable.type_name);
    }

    // Exit scope
    semantic_analyzer_exit_scope(analyzer);
    printf("✓ Exited local scope\n");
    printf("  Current scope level: %d\n", analyzer->current_scope->scope_level);

    // Try to find local variable (should fail)
    Symbol* not_found_local = symbol_table_lookup(analyzer->current_scope, "local");
    printf("Found local variable after exiting scope: %s\n", not_found_local ? "YES (WRONG)" : "NO (CORRECT)");

    // Check conditions
    int condition1 = (analyzer->current_scope->scope_level == 0);
    int condition2 = (found_local == local_var);
    int condition3 = (found_global_in_local == var1);
    int condition4 = (not_found_local == NULL);

    printf("\n=== CONDITION CHECKS ===\n");
    printf("Condition 1 (scope level == 0): %s\n", condition1 ? "PASS" : "FAIL");
    printf("Condition 2 (found_local == local_var): %s\n", condition2 ? "PASS" : "FAIL");
    printf("Condition 3 (found_global_in_local == var1): %s\n", condition3 ? "PASS" : "FAIL");
    printf("Condition 4 (not_found_local == NULL): %s\n", condition4 ? "PASS" : "FAIL");

    if (condition1 && condition2 && condition3 && condition4) {
        printf("\n🎉 ALL CONDITIONS PASSED!\n");
        semantic_analyzer_free(analyzer);
        return 0;
    } else {
        printf("\n❌ SOME CONDITIONS FAILED!\n");
        semantic_analyzer_free(analyzer);
        return 1;
    }
}