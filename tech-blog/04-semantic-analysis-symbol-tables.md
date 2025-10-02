# 语义分析与符号表

## 语义分析概述

语义分析是编译器的第三个阶段，它在语法分析的基础上进行更深层次的检查。如果说语法分析确保程序"看起来正确"，那么语义分析确保程序"意思正确"。语义分析主要负责类型检查、作用域分析、变量声明检查等任务。

### 语义分析的核心任务

- **类型检查**：确保操作符合类型兼容性要求
- **作用域分析**：验证标识符的声明和使用是否在正确的作用域内
- **变量声明检查**：确保变量在使用前已声明
- **函数调用检查**：验证函数调用的参数数量和类型
- **控制流检查**：检查break、continue、return等语句的使用
- **常量折叠**：在编译时计算常量表达式的值
- **符号表构建**：维护程序中所有符号的信息

## 符号表（Symbol Table）

符号表是语义分析的核心数据结构，用于存储程序中标识符（变量、函数、类型等）的信息。

### 符号表的基本结构

```c
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER,
    SYMBOL_TYPE,
    SYMBOL_CONSTANT
} SymbolKind;

typedef struct TypeInfo {
    TypeCategory category;    // 基础类型、数组、指针、结构体等
    const char* type_name;    // 类型名称
    int size;                // 类型大小（字节）
    bool is_signed;          // 是否有符号
    struct TypeInfo* base_type; // 基础类型（用于数组、指针）
    int array_size;          // 数组大小
    struct FieldInfo* fields; // 结构体字段
} TypeInfo;

typedef struct SymbolEntry {
    const char* name;        // 符号名称
    SymbolKind kind;         // 符号类型
    TypeInfo* type;          // 类型信息
    int scope_level;         // 作用域层级
    bool is_defined;         // 是否已定义
    bool is_used;            // 是否被使用
    int line_number;         // 声明位置
    struct SymbolEntry* next; // 哈希表链表指针
    union {
        struct {
            int offset;      // 变量在栈中的偏移
            bool is_parameter; // 是否为参数
        } var_info;
        struct {
            struct SymbolEntry* parameters; // 参数列表
            int param_count;   // 参数数量
            struct SymbolEntry* local_symbols; // 局部符号
            TypeInfo* return_type; // 返回类型
        } func_info;
        struct {
            int value;        // 常量值
        } const_info;
    } details;
} SymbolEntry;

typedef struct Scope {
    int level;               // 作用域层级
    SymbolEntry* symbols;    // 该作用域的符号表
    struct Scope* parent;    // 父作用域指针
    struct Scope* child;     // 子作用域链表
    struct Scope* sibling;   // 兄弟作用域
} Scope;
```

### 符号表的实现

#### 哈希表实现

```c
#define TABLE_SIZE 211

typedef struct SymbolTable {
    SymbolEntry* buckets[TABLE_SIZE];  // 哈希桶
    Scope* current_scope;              // 当前作用域
    Scope* global_scope;               // 全局作用域
} SymbolTable;

// 哈希函数
unsigned int hash(const char* name) {
    unsigned int hash_value = 0;
    while (*name) {
        hash_value = hash_value * 31 + *name++;
    }
    return hash_value % TABLE_SIZE;
}

// 查找符号
SymbolEntry* lookupSymbol(SymbolTable* table, const char* name) {
    unsigned int index = hash(name);
    SymbolEntry* entry = table->buckets[index];

    while (entry != NULL) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}

// 查找符号（考虑作用域）
SymbolEntry* lookupSymbolInScope(SymbolTable* table, const char* name, Scope* scope) {
    Scope* current = scope;

    while (current != NULL) {
        SymbolEntry* entry = lookupSymbolInCurrentScope(current, name);
        if (entry != NULL) {
            return entry;
        }
        current = current->parent;
    }

    return NULL;
}

// 插入符号
bool insertSymbol(SymbolTable* table, SymbolEntry* entry) {
    unsigned int index = hash(entry->name);

    // 检查是否已存在同名符号
    SymbolEntry* existing = lookupSymbolInCurrentScope(table->current_scope, entry->name);
    if (existing != NULL) {
        return false; // 符号重复定义
    }

    // 插入到哈希表头部
    entry->next = table->buckets[index];
    table->buckets[index] = entry;

    // 链接到当前作用域
    entry->scope_level = table->current_scope->level;

    return true;
}
```

#### 作用域管理

```c
// 进入新作用域
void enterScope(SymbolTable* table) {
    Scope* new_scope = malloc(sizeof(Scope));
    new_scope->level = table->current_scope->level + 1;
    new_scope->symbols = NULL;
    new_scope->parent = table->current_scope;
    new_scope->child = NULL;
    new_scope->sibling = table->current_scope->child;
    table->current_scope->child = new_scope;
    table->current_scope = new_scope;
}

// 离开当前作用域
void exitScope(SymbolTable* table) {
    if (table->current_scope != table->global_scope) {
        table->current_scope = table->current_scope->parent;
    }
}

// 在当前作用域查找符号
SymbolEntry* lookupSymbolInCurrentScope(Scope* scope, const char* name) {
    SymbolEntry* entry = scope->symbols;
    while (entry != NULL) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}
```

## 类型系统

类型系统是语义分析的核心，它定义了程序中各种类型的规则和操作。

### 类型表示

```c
typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_ARRAY,
    TYPE_POINTER,
    TYPE_STRUCT,
    TYPE_FUNCTION
} TypeCategory;

// 类型创建函数
TypeInfo* createBasicType(TypeCategory category) {
    TypeInfo* type = malloc(sizeof(TypeInfo));
    type->category = category;
    type->size = getTypeSize(category);
    type->base_type = NULL;
    type->fields = NULL;
    return type;
}

TypeInfo* createArrayType(TypeInfo* base_type, int size) {
    TypeInfo* type = malloc(sizeof(TypeInfo));
    type->category = TYPE_ARRAY;
    type->base_type = base_type;
    type->array_size = size;
    type->size = base_type->size * size;
    return type;
}

TypeInfo* createPointerType(TypeInfo* base_type) {
    TypeInfo* type = malloc(sizeof(TypeInfo));
    type->category = TYPE_POINTER;
    type->base_type = base_type;
    type->size = sizeof(void*);  // 指针大小
    return type;
}

TypeInfo* createStructType(const char* name, FieldInfo* fields) {
    TypeInfo* type = malloc(sizeof(TypeInfo));
    type->category = TYPE_STRUCT;
    type->type_name = strdup(name);
    type->fields = fields;

    // 计算结构体大小
    int total_size = 0;
    FieldInfo* field = fields;
    while (field != NULL) {
        total_size += field->type->size;
        field = field->next;
    }
    type->size = total_size;

    return type;
}
```

### 类型检查

#### 类型兼容性检查

```c
// 检查类型是否兼容
bool areTypesCompatible(TypeInfo* type1, TypeInfo* type2) {
    // 完全相同的类型
    if (type1->category == type2->category) {
        switch (type1->category) {
            case TYPE_INT:
            case TYPE_FLOAT:
            case TYPE_CHAR:
            case TYPE_BOOL:
                return true;
            case TYPE_ARRAY:
                return areTypesCompatible(type1->base_type, type2->base_type) &&
                       type1->array_size == type2->array_size;
            case TYPE_POINTER:
                return areTypesCompatible(type1->base_type, type2->base_type);
            case TYPE_STRUCT:
                return strcmp(type1->type_name, type2->type_name) == 0;
        }
    }

    // 数值类型之间的隐式转换
    if (isNumericType(type1) && isNumericType(type2)) {
        return true;
    }

    // 数组到指针的转换
    if (type1->category == TYPE_POINTER && type2->category == TYPE_ARRAY) {
        return areTypesCompatible(type1->base_type, type2->base_type);
    }

    return false;
}

// 获取公共类型（用于类型提升）
TypeInfo* getCommonType(TypeInfo* type1, TypeInfo* type2) {
    // 如果类型相同，返回该类型
    if (areTypesCompatible(type1, type2)) {
        if (type1->category >= type2->category) {
            return type1;
        } else {
            return type2;
        }
    }

    // 数值类型提升
    if (isNumericType(type1) && isNumericType(type2)) {
        if (type1->category == TYPE_FLOAT || type2->category == TYPE_FLOAT) {
            return createBasicType(TYPE_FLOAT);
        }
        return createBasicType(TYPE_INT);
    }

    return NULL; // 无公共类型
}

bool isNumericType(TypeInfo* type) {
    return type->category == TYPE_INT ||
           type->category == TYPE_FLOAT ||
           type->category == TYPE_CHAR;
}
```

## 语义分析器实现

### 语义分析器结构

```c
typedef struct SemanticAnalyzer {
    SymbolTable* symbol_table;
    TypeInfo* current_function_return_type;
    bool has_return_statement;
    int error_count;
    int warning_count;
} SemanticAnalyzer;

// 创建语义分析器
SemanticAnalyzer* createSemanticAnalyzer() {
    SemanticAnalyzer* analyzer = malloc(sizeof(SemanticAnalyzer));
    analyzer->symbol_table = createSymbolTable();
    analyzer->current_function_return_type = NULL;
    analyzer->has_return_statement = false;
    analyzer->error_count = 0;
    analyzer->warning_count = 0;

    // 初始化全局作用域
    enterScope(analyzer->symbol_table);

    return analyzer;
}
```

### AST遍历和语义检查

```c
// 主分析函数
void analyze(ASTNode* node, SemanticAnalyzer* analyzer) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_PROGRAM:
            analyzeProgram(node, analyzer);
            break;
        case NODE_FUNCTION_DECL:
            analyzeFunctionDecl(node, analyzer);
            break;
        case NODE_DECL_STMT:
            analyzeDeclStmt(node, analyzer);
            break;
        case NODE_ASSIGN_STMT:
            analyzeAssignStmt(node, analyzer);
            break;
        case NODE_BINARY_EXPR:
            analyzeBinaryExpr(node, analyzer);
            break;
        case NODE_CALL_EXPR:
            analyzeCallExpr(node, analyzer);
            break;
        case NODE_RETURN_STMT:
            analyzeReturnStmt(node, analyzer);
            break;
        case NODE_IF_STMT:
            analyzeIfStmt(node, analyzer);
            break;
        case NODE_WHILE_STMT:
            analyzeWhileStmt(node, analyzer);
            break;
        default:
            // 递归分析子节点
            for (int i = 0; i < node->child_count; i++) {
                analyze(node->children[i], analyzer);
            }
            break;
    }
}
```

### 具体语义检查实现

#### 变量声明检查

```c
void analyzeDeclStmt(ASTNode* node, SemanticAnalyzer* analyzer) {
    // 获取类型信息
    TypeInfo* type = analyzeType(node->children[0], analyzer);
    if (type == NULL) {
        semanticError(node->line, "无效的类型");
        return;
    }

    // 处理多个变量声明
    ASTNode* declarators = node->children[1];
    for (int i = 0; i < declarators->child_count; i++) {
        ASTNode* declarator = declarators->children[i];
        const char* name = declarator->children[0]->value;

        // 检查是否已声明
        SymbolEntry* existing = lookupSymbolInScope(
            analyzer->symbol_table->current_scope, name);
        if (existing != NULL) {
            semanticError(node->line, "变量 '%s' 重复定义", name);
            continue;
        }

        // 创建符号表条目
        SymbolEntry* entry = createSymbolEntry(name, SYMBOL_VARIABLE);
        entry->type = type;
        entry->line_number = node->line;

        // 如果有初始化表达式，检查类型兼容性
        if (declarator->child_count > 1) {
            TypeInfo* init_type = analyzeExpression(
                declarator->children[1], analyzer);
            if (!areTypesCompatible(type, init_type)) {
                semanticError(node->line,
                    "初始化表达式类型与变量类型不兼容");
            }
        }

        // 插入符号表
        if (!insertSymbol(analyzer->symbol_table, entry)) {
            semanticError(node->line, "无法插入符号表");
        }
    }
}
```

#### 表达式类型检查

```c
TypeInfo* analyzeExpression(ASTNode* node, SemanticAnalyzer* analyzer) {
    if (node == NULL) return NULL;

    switch (node->type) {
        case NODE_VARIABLE: {
            SymbolEntry* entry = lookupSymbolInScope(
                analyzer->symbol_table->current_scope, node->value);
            if (entry == NULL) {
                semanticError(node->line, "未声明的变量 '%s'", node->value);
                return NULL;
            }
            entry->is_used = true;
            return entry->type;
        }

        case NODE_CONSTANT: {
            if (node->value[0] == '"') {
                return createPointerType(createBasicType(TYPE_CHAR));
            } else if (strchr(node->value, '.') != NULL) {
                return createBasicType(TYPE_FLOAT);
            } else {
                return createBasicType(TYPE_INT);
            }
        }

        case NODE_BINARY_EXPR: {
            TypeInfo* left_type = analyzeExpression(node->children[0], analyzer);
            TypeInfo* right_type = analyzeExpression(node->children[1], analyzer);

            if (left_type == NULL || right_type == NULL) {
                return NULL;
            }

            // 赋值操作
            if (strcmp(node->value, "=") == 0) {
                if (!areTypesCompatible(left_type, right_type)) {
                    semanticError(node->line, "赋值操作类型不兼容");
                }
                return left_type;
            }

            // 算术运算
            if (strcmp(node->value, "+") == 0 ||
                strcmp(node->value, "-") == 0 ||
                strcmp(node->value, "*") == 0 ||
                strcmp(node->value, "/") == 0) {

                if (!isNumericType(left_type) || !isNumericType(right_type)) {
                    semanticError(node->line, "算术运算需要数值类型");
                    return NULL;
                }

                return getCommonType(left_type, right_type);
            }

            // 比较运算
            if (strcmp(node->value, "==") == 0 ||
                strcmp(node->value, "!=") == 0 ||
                strcmp(node->value, "<") == 0 ||
                strcmp(node->value, ">") == 0 ||
                strcmp(node->value, "<=") == 0 ||
                strcmp(node->value, ">=") == 0) {

                if (!areTypesCompatible(left_type, right_type)) {
                    semanticError(node->line, "比较运算类型不兼容");
                }

                return createBasicType(TYPE_BOOL);
            }

            return NULL;
        }

        case NODE_CALL_EXPR: {
            return analyzeCallExpr(node, analyzer);
        }

        default:
            return NULL;
    }
}
```

#### 函数声明和调用检查

```c
void analyzeFunctionDecl(ASTNode* node, SemanticAnalyzer* analyzer) {
    const char* name = node->value;
    TypeInfo* return_type = analyzeType(node->children[0], analyzer);

    // 创建函数符号
    SymbolEntry* entry = createSymbolEntry(name, SYMBOL_FUNCTION);
    entry->type = return_type;
    entry->details.func_info.return_type = return_type;
    entry->line_number = node->line;

    // 检查是否已声明
    SymbolEntry* existing = lookupSymbolInScope(
        analyzer->symbol_table->current_scope, name);
    if (existing != NULL) {
        semanticError(node->line, "函数 '%s' 重复定义", name);
        return;
    }

    // 插入函数符号
    insertSymbol(analyzer->symbol_table, entry);

    // 进入函数作用域
    enterScope(analyzer->symbol_table);
    analyzer->current_function_return_type = return_type;
    analyzer->has_return_statement = false;

    // 分析参数
    if (node->children[1] != NULL) {
        analyzeParameters(node->children[1], analyzer, entry);
    }

    // 分析函数体
    analyze(node->children[2], analyzer);

    // 检查非void函数是否有返回语句
    if (return_type->category != TYPE_VOID && !analyzer->has_return_statement) {
        semanticWarning(node->line,
            "函数 '%s' 缺少返回语句", name);
    }

    // 离开函数作用域
    exitScope(analyzer->symbol_table);
    analyzer->current_function_return_type = NULL;
}

TypeInfo* analyzeCallExpr(ASTNode* node, SemanticAnalyzer* analyzer) {
    const char* name = node->children[0]->value;

    // 查找函数声明
    SymbolEntry* entry = lookupSymbolInScope(
        analyzer->symbol_table->current_scope, name);
    if (entry == NULL || entry->kind != SYMBOL_FUNCTION) {
        semanticError(node->line, "未声明的函数 '%s'", name);
        return NULL;
    }

    // 检查参数数量
    int arg_count = node->child_count - 1;
    int param_count = entry->details.func_info.param_count;

    if (arg_count != param_count) {
        semanticError(node->line,
            "函数 '%s' 参数数量不匹配，期望 %d 个，实际 %d 个",
            name, param_count, arg_count);
        return entry->details.func_info.return_type;
    }

    // 检查参数类型
    SymbolEntry* param = entry->details.func_info.parameters;
    for (int i = 0; i < arg_count; i++) {
        TypeInfo* arg_type = analyzeExpression(node->children[i + 1], analyzer);
        if (!areTypesCompatible(param->type, arg_type)) {
            semanticError(node->line,
                "函数 '%s' 参数 %d 类型不匹配", name, i + 1);
        }
        param = param->next;
    }

    return entry->details.func_info.return_type;
}
```

## 控制流检查

### Return语句检查

```c
void analyzeReturnStmt(ASTNode* node, SemanticAnalyzer* analyzer) {
    analyzer->has_return_statement = true;

    TypeInfo* return_type = NULL;
    if (node->child_count > 0) {
        return_type = analyzeExpression(node->children[0], analyzer);
    } else {
        return_type = createBasicType(TYPE_VOID);
    }

    if (analyzer->current_function_return_type == NULL) {
        semanticError(node->line, "return语句不能在函数外部使用");
        return;
    }

    if (!areTypesCompatible(analyzer->current_function_return_type, return_type)) {
        semanticError(node->line, "返回值类型与函数声明类型不匹配");
    }
}
```

### Break和Continue检查

```c
void analyzeBreakStmt(ASTNode* node, SemanticAnalyzer* analyzer) {
    if (!isInsideLoop(analyzer)) {
        semanticError(node->line, "break语句只能在循环内使用");
    }
}

void analyzeContinueStmt(ASTNode* node, SemanticAnalyzer* analyzer) {
    if (!isInsideLoop(analyzer)) {
        semanticError(node->line, "continue语句只能在循环内使用");
    }
}
```

## 常量折叠

常量折叠是在编译时计算常量表达式的值，以提高运行时性能。

```c
TypeInfo* performConstantFolding(ASTNode* node, SemanticAnalyzer* analyzer) {
    if (node->type != NODE_BINARY_EXPR) {
        return analyzeExpression(node, analyzer);
    }

    // 分析左右操作数
    TypeInfo* left_type = analyzeExpression(node->children[0], analyzer);
    TypeInfo* right_type = analyzeExpression(node->children[1], analyzer);

    // 如果两个操作数都是常量，进行折叠
    if (isConstant(node->children[0]) && isConstant(node->children[1])) {
        if (isNumericType(left_type) && isNumericType(right_type)) {
            return foldNumericConstants(node);
        }
    }

    return getCommonType(left_type, right_type);
}

TypeInfo* foldNumericConstants(ASTNode* node) {
    const char* left_val = node->children[0]->value;
    const char* right_val = node->children[1]->value;
    const char* op = node->value;

    double left_num = atof(left_val);
    double right_num = atof(right_val);
    double result;

    if (strcmp(op, "+") == 0) {
        result = left_num + right_num;
    } else if (strcmp(op, "-") == 0) {
        result = left_num - right_num;
    } else if (strcmp(op, "*") == 0) {
        result = left_num * right_num;
    } else if (strcmp(op, "/") == 0) {
        if (right_num == 0) {
            semanticError(node->line, "除零错误");
            return NULL;
        }
        result = left_num / right_num;
    } else {
        return NULL; // 不支持的运算
    }

    // 将结果转换回字符串
    char result_str[64];
    if (strchr(left_val, '.') != NULL || strchr(right_val, '.') != NULL) {
        snprintf(result_str, sizeof(result_str), "%.6g", result);
        node->type = NODE_CONSTANT;
        node->value = strdup(result_str);
        return createBasicType(TYPE_FLOAT);
    } else {
        snprintf(result_str, sizeof(result_str), "%d", (int)result);
        node->type = NODE_CONSTANT;
        node->value = strdup(result_str);
        return createBasicType(TYPE_INT);
    }
}

bool isConstant(ASTNode* node) {
    return node->type == NODE_CONSTANT;
}
```

## 错误处理和报告

### 错误报告函数

```c
void semanticError(int line, const char* format, ...) {
    va_list args;
    va_start(args, format);

    fprintf(stderr, "语义错误 (第%d行): ", line);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
    analyzer->error_count++;
}

void semanticWarning(int line, const char* format, ...) {
    va_list args;
    va_start(args, format);

    fprintf(stderr, "警告 (第%d行): ", line);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");

    va_end(args);
    analyzer->warning_count++;
}
```

### 未使用变量检查

```c
void checkUnusedVariables(Scope* scope) {
    SymbolEntry* entry = scope->symbols;
    while (entry != NULL) {
        if (entry->kind == SYMBOL_VARIABLE && !entry->is_used) {
            semanticWarning(entry->line_number,
                "变量 '%s' 声明但未使用", entry->name);
        }
        entry = entry->next;
    }

    // 递归检查子作用域
    Scope* child = scope->child;
    while (child != NULL) {
        checkUnusedVariables(child);
        child = child->sibling;
    }
}
```

## 实际案例分析

### 示例代码

```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 10;
    int y = 20;
    int z = add(x, y);
    return z;
}
```

### 语义分析过程

1. **全局符号表构建**：
   - 函数 `add`：类型 `int(int, int)`
   - 函数 `main`：类型 `int()`

2. **函数 `add` 分析**：
   - 参数 `a`：类型 `int`
   - 参数 `b`：类型 `int`
   - 返回语句：`a + b` 类型检查通过

3. **函数 `main` 分析**：
   - 变量 `x`：类型 `int`，初始化值 `10`
   - 变量 `y`：类型 `int`，初始化值 `20`
   - 函数调用 `add(x, y)`：参数类型匹配
   - 变量 `z`：类型 `int`，接收函数返回值
   - 返回语句：类型检查通过

### 错误示例及检测

```c
// 错误示例1：未声明的变量
int main() {
    x = 10;  // 错误：变量 x 未声明
    return x;
}

// 错误示例2：类型不匹配
int main() {
    int x = "hello";  // 错误：类型不匹配
    return x;
}

// 错误示例3：函数调用参数不匹配
int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(10);  // 错误：参数数量不匹配
    return result;
}
```

## 高级话题

### 类型推断

对于支持类型推断的语言，需要实现类型推断算法：

```c
TypeInfo* inferType(ASTNode* node, SemanticAnalyzer* analyzer) {
    switch (node->type) {
        case NODE_VARIABLE:
            return inferVariableType(node, analyzer);
        case NODE_BINARY_EXPR:
            return inferBinaryExprType(node, analyzer);
        case NODE_CALL_EXPR:
            return inferCallExprType(node, analyzer);
        default:
            return NULL;
    }
}
```

### 泛型类型支持

支持泛型编程需要更复杂的类型系统：

```c
typedef struct GenericType {
    const char* name;
    TypeInfo* constraint;
    struct GenericType* type_params;
} GenericType;

typedef struct TypeVariable {
    const char* name;
    TypeInfo* instance;
    int level;
} TypeVariable;
```

### 重载函数解析

```c
SymbolEntry* resolveOverload(SymbolTable* table, const char* name,
                           TypeInfo* arg_types[], int arg_count) {
    SymbolEntry* best_match = NULL;
    int best_score = -1;

    // 查找所有同名函数
    SymbolEntry* entry = lookupAllSymbols(table, name);
    while (entry != NULL && entry->kind == SYMBOL_FUNCTION) {
        int score = calculateOverloadScore(entry, arg_types, arg_count);
        if (score > best_score) {
            best_score = score;
            best_match = entry;
        }
        entry = entry->next_overload;
    }

    return best_match;
}
```

## 总结

语义分析是编译器中确保程序语义正确性的关键阶段。它通过类型检查、作用域分析、变量声明检查等机制，在编译早期发现程序中的语义错误。

一个完整的语义分析器应该具备：

1. **完整的类型系统**：支持基本类型、复合类型和用户自定义类型
2. **灵活的作用域管理**：正确处理嵌套作用域和符号可见性
3. **全面的错误检查**：检测各种语义错误并提供有用的错误信息
4. **性能优化**：包括常量折叠、死代码消除等优化
5. **良好的错误恢复**：在发现错误时能够继续分析

理解语义分析的工作原理有助于我们设计类型安全的编程语言，构建更健壮的编译器，并编写更易于理解和维护的代码。在下一篇文章中，我们将探讨中间代码生成，如何将AST转换为独立于目标机器的中间表示。