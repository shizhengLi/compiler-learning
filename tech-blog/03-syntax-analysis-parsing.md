# 语法分析与解析

## 语法分析概述

语法分析（Syntax Analysis）是编译器的第二个阶段，也称为解析（Parsing）。它接收词法分析器产生的标记序列，验证这些标记是否符合编程语言的语法规则，并构建抽象语法树（Abstract Syntax Tree, AST）。

### 语法分析的核心任务

- **语法验证**：检查标记序列是否符合语法规则
- **结构构建**：构建层次化的语法结构（AST）
- **错误恢复**：在发现语法错误时尝试恢复并继续分析
- **语法树构建**：为后续的语义分析和代码生成提供结构化输入

## 上下文无关文法（CFG）

语法分析的基础是上下文无关文法，它由四个组成部分：

### 文法的形式化定义

G = (V, T, P, S)

- **V**：非终结符集合（变量）
- **T**：终结符集合（标记）
- **P**：产生式规则集合
- **S**：起始符号

### 示例：简单算术表达式的文法

```
E → E + T | E - T | T
T → T * F | T / F | F
F → ( E ) | num | id
```

这个文法可以生成如下表达式：
- `num + num`
- `id * (num + id)`
- `num - num * num`

## 抽象语法树（AST）

抽象语法树是程序结构的层次化表示，它去除了语法中的冗余信息，保留了程序的语义结构。

### 语法分析树 vs 抽象语法树

**语法分析树**：
```
        E
       /|\
      E + T
      |   |
      T   F
      |   |
      F   num
      |
      num
```

**抽象语法树**：
```
        +
       / \
    num  num
```

### AST节点类型定义

```c
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    NODE_PARAM_LIST,
    NODE_COMPOUND_STMT,
    NODE_DECL_STMT,
    NODE_ASSIGN_STMT,
    NODE_RETURN_STMT,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_CALL_EXPR,
    NODE_VARIABLE,
    NODE_CONSTANT
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char* value;
    int line;
    struct ASTNode** children;
    int child_count;
    struct ASTNode* next;  // 用于兄弟节点链接
    // 类型信息（语义分析阶段添加）
    TypeInfo* type_info;
} ASTNode;
```

## 语法分析技术

### 自顶向下分析（Top-Down Parsing）

自顶向下分析从文法的起始符号开始，尝试推导出输入的标记序列。

#### 递归下降分析器

递归下降分析器是最直观的自顶向下分析方法：

```c
// 函数声明解析
ASTNode* parseFunctionDecl() {
    ASTNode* node = createNode(NODE_FUNCTION_DECL);

    // 解析返回类型
    node->children[0] = parseType();

    // 解析函数名
    Token token = match(TOKEN_IDENTIFIER);
    node->value = strdup(token.lexeme);

    // 解析参数列表
    match(TOKEN_LEFT_PAREN);
    node->children[1] = parseParamList();
    match(TOKEN_RIGHT_PAREN);

    // 解析函数体
    node->children[2] = parseCompoundStmt();

    return node;
}

// 表达式解析（处理优先级）
ASTNode* parseExpression() {
    return parseAssignmentExpr();
}

ASTNode* parseAssignmentExpr() {
    ASTNode* left = parseEqualityExpr();

    if (currentToken.type == TOKEN_ASSIGN) {
        ASTNode* node = createNode(NODE_ASSIGN_EXPR);
        node->children[0] = left;
        match(TOKEN_ASSIGN);
        node->children[1] = parseAssignmentExpr();
        return node;
    }

    return left;
}

ASTNode* parseEqualityExpr() {
    ASTNode* left = parseRelationalExpr();

    while (currentToken.type == TOKEN_EQUAL ||
           currentToken.type == TOKEN_NOT_EQUAL) {
        ASTNode* node = createNode(NODE_BINARY_EXPR);
        node->value = strdup(currentToken.lexeme);
        node->children[0] = left;

        match(currentToken.type);
        node->children[1] = parseRelationalExpr();
        left = node;
    }

    return left;
}
```

#### 左递归消除

直接左递归会导致递归下降分析器无限递归，需要消除：

**原始文法**：
```
E → E + T | T
```

**消除左递归后的文法**：
```
E → T E'
E' → + T E' | ε
```

**对应的代码**：
```c
ASTNode* parseExpr() {
    ASTNode* left = parseTerm();
    return parseExprPrime(left);
}

ASTNode* parseExprPrime(ASTNode* left) {
    if (currentToken.type == TOKEN_PLUS) {
        match(TOKEN_PLUS);
        ASTNode* right = parseTerm();
        ASTNode* node = createBinaryNode("+", left, right);
        return parseExprPrime(node);
    }
    return left;
}
```

#### FIRST集和FOLLOW集

为了构建预测分析表，需要计算FIRST集和FOLLOW集：

```c
// FIRST集计算
Set* calculateFirst(Production prod) {
    Set* first = createSet();

    for (int i = 0; i < prod.symbol_count; i++) {
        Symbol* sym = prod.symbols[i];

        if (sym->is_terminal) {
            addToSet(first, sym);
            break;
        } else {
            Set* sym_first = getFirstSet(sym);
            unionSets(first, sym_first);

            if (!containsEpsilon(sym_first)) {
                removeEpsilon(first);
                break;
            }
        }
    }

    return first;
}

// FOLLOW集计算
void calculateFollowSets() {
    addToSet(followSets[startSymbol], EOF_TOKEN);

    bool changed;
    do {
        changed = false;

        for (Production prod : productions) {
            for (int i = 0; i < prod.symbol_count; i++) {
                Symbol* B = prod.symbols[i];
                if (B->is_terminal) continue;

                // 计算FIRST(beta)
                Set* first_beta = createSet();
                for (int j = i + 1; j < prod.symbol_count; j++) {
                    Symbol* beta = prod.symbols[j];
                    if (beta->is_terminal) {
                        addToSet(first_beta, beta);
                        break;
                    } else {
                        unionSets(first_beta, getFirstSet(beta));
                        if (!containsEpsilon(getFirstSet(beta))) {
                            removeEpsilon(first_beta);
                            break;
                        }
                    }
                }

                // 添加FIRST(beta) - {epsilon}到FOLLOW(B)
                Set* old_follow = followSets[B];
                Set* new_follow = copySet(old_follow);
                unionSets(new_follow, first_beta);
                removeEpsilon(new_follow);

                if (!setsEqual(old_follow, new_follow)) {
                    followSets[B] = new_follow;
                    changed = true;
                }

                // 如果epsilon在FIRST(beta)中，添加FOLLOW(A)到FOLLOW(B)
                if (i == prod.symbol_count - 1 ||
                    containsEpsilon(first_beta)) {
                    Set* follow_A = followSets[prod.lhs];
                    if (!isSubset(follow_A, followSets[B])) {
                        unionSets(followSets[B], follow_A);
                        changed = true;
                    }
                }
            }
        }
    } while (changed);
}
```

### 自底向上分析（Bottom-Up Parsing）

自底向上分析从输入的标记序列开始，反向推导到文法的起始符号。

#### LR分析器

LR分析器是最强大的自底向上分析方法，包括：
- **SLR(1)**：简单LR分析
- **LR(1)**：规范LR分析
- **LALR(1)**：向前看LR分析（如YACC/BISON）

##### LR分析表结构

```
状态 |   ID    |   +    |   $    |   E    |   T    |   F    |   E'   |   T'
-----|---------|--------|--------|--------|--------|--------|--------|--------
 0   |   s5    |        |        |   1    |   2    |   3    |        |
 1   |         |   s6   |  acc   |        |        |        |        |
 2   |         |  r2    |  r2    |        |        |        |        |  r2
 3   |         |  r4    |  r4    |        |        |        |        |  r4
 4   |         |   s6   |  acc   |        |        |        |        |
 5   |         |  r6    |  r6    |        |        |        |        |  r6
 6   |   s5    |        |        |        |   2    |   3    |        |
 7   |   s5    |        |        |        |   2    |   3    |        |   8
```

##### LR分析算法

```c
typedef struct {
    int state;
    Stack* symbol_stack;  // 符号栈
    Stack* state_stack;   // 状态栈
} LRParser;

bool parseLR(LRParser* parser, Token* tokens) {
    pushStack(parser->state_stack, 0);  // 初始状态

    int token_index = 0;
    Token current_token = tokens[token_index];

    while (true) {
        int current_state = topStack(parser->state_stack);
        Action action = getAction(current_state, current_token.type);

        switch (action.type) {
            case SHIFT:
                pushStack(parser->symbol_stack, current_token);
                pushStack(parser->state_stack, action.next_state);
                current_token = tokens[++token_index];
                break;

            case REDUCE:
                Production prod = getProduction(action.production_num);

                // 弹出产生式右部的符号和状态
                for (int i = 0; i < prod.rhs_length; i++) {
                    popStack(parser->symbol_stack);
                    popStack(parser->state_stack);
                }

                // 推入产生式左部的非终结符
                Symbol lhs = prod.lhs;
                pushStack(parser->symbol_stack, lhs);

                // 获取新的状态并推入
                int new_state = topStack(parser->state_stack);
                int goto_state = getGoto(new_state, lhs);
                pushStack(parser->state_stack, goto_state);
                break;

            case ACCEPT:
                return true;  // 分析成功

            case ERROR:
                reportSyntaxError(current_token);
                return false;  // 分析失败
        }
    }
}
```

## 错误恢复机制

### 恐慌模式（Panic Mode）

当遇到错误时，跳过输入直到找到同步标记：

```c
void panicModeRecovery() {
    // 同步标记集合
    TokenType sync_tokens[] = {
        TOKEN_SEMICOLON, TOKEN_RIGHT_BRACE,
        TOKEN_RIGHT_PAREN, TOKEN_EOF
    };

    // 跳过标记直到找到同步标记
    while (!isSyncToken(currentToken.type, sync_tokens,
                       sizeof(sync_tokens)/sizeof(TokenType))) {
        currentToken = getNextToken();
    }

    if (currentToken.type != TOKEN_EOF) {
        currentToken = getNextToken();  // 跳过同步标记
    }
}
```

### 短语级别恢复

通过简单的替换、插入或删除来修正错误：

```c
bool phraseLevelRecovery() {
    // 尝试插入缺失的分号
    if (canInsertSemicolon()) {
        insertToken(TOKEN_SEMICOLON);
        return true;
    }

    // 尝试删除多余的标记
    if (canDeleteToken()) {
        currentToken = getNextToken();
        return true;
    }

    // 尝试替换标记
    if (canReplaceToken()) {
        TokenType replacement = suggestReplacement();
        currentToken.type = replacement;
        return true;
    }

    return false;
}
```

## 实际案例分析

### 解析简单函数

让我们分析如何解析以下函数：

```c
int add(int a, int b) {
    return a + b;
}
```

#### 标记序列

```
TOKEN_INT("int")
TOKEN_IDENTIFIER("add")
TOKEN_LEFT_PAREN
TOKEN_INT("int")
TOKEN_IDENTIFIER("a")
TOKEN_COMMA
TOKEN_INT("int")
TOKEN_IDENTIFIER("b")
TOKEN_RIGHT_PAREN
TOKEN_LEFT_BRACE
TOKEN_RETURN("return")
TOKEN_IDENTIFIER("a")
TOKEN_PLUS
TOKEN_IDENTIFIER("b")
TOKEN_SEMICOLON
TOKEN_RIGHT_BRACE
```

#### 构建的AST

```
FunctionDecl: add
├── ReturnType: int
├── Parameters
│   ├── ParamDecl: a (int)
│   └── ParamDecl: b (int)
└── Body
    └── ReturnStmt
        └── BinaryExpr: +
            ├── Variable: a
            └── Variable: b
```

### 解析复杂表达式

解析表达式：`a * (b + c) / d - e`

#### 运算符优先级处理

```c
// 处理优先级的递归下降解析器
ASTNode* parseExpression() {
    return parseSubtraction();
}

ASTNode* parseSubtraction() {
    ASTNode* left = parseAddition();

    while (currentToken.type == TOKEN_MINUS) {
        Token op = currentToken;
        match(TOKEN_MINUS);
        ASTNode* right = parseAddition();
        left = createBinaryNode(op.lexeme, left, right);
    }

    return left;
}

ASTNode* parseAddition() {
    ASTNode* left = parseMultiplication();

    while (currentToken.type == TOKEN_PLUS) {
        Token op = currentToken;
        match(TOKEN_PLUS);
        ASTNode* right = parseMultiplication();
        left = createBinaryNode(op.lexeme, left, right);
    }

    return left;
}

// ... 继续处理乘除法
```

#### 最终AST结构

```
        -
       / \
      *   e
     / \
    a   /
       / \
      +   d
     / \
    b   c
```

## 语法分析器生成工具

### YACC/BISON

YACC（Yet Another Compiler-Compiler）是经典的语法分析器生成器：

```yacc
%{
#include "ast.h"
#include "lexer.h"
%}

// 优先级和结合性
%left PLUS MINUS
%left MULTIPLY DIVIDE
%right UMINUS

%token INT IDENTIFIER
%left LEFT_PAREN RIGHT_PAREN

%%

program: stmt_list
        { $$ = createProgramNode($1); }
        ;

stmt_list: stmt_list stmt
         { $$ = addStatement($1, $2); }
         | stmt
         { $$ = createStmtList($1); }
         ;

stmt: expr_stmt
    | compound_stmt
    | return_stmt
    ;

expr_stmt: expression SEMICOLON
         { $$ = createExprStmt($1); }
         ;

expression: expression PLUS expression
          { $$ = createBinaryNode("+", $1, $3); }
          | expression MINUS expression
          { $$ = createBinaryNode("-", $1, $3); }
          | expression MULTIPLY expression
          { $$ = createBinaryNode("*", $1, $3); }
          | expression DIVIDE expression
          { $$ = createBinaryNode("/", $1, $3); }
          | MINUS expression %prec UMINUS
          { $$ = createUnaryNode("-", $2); }
          | LEFT_PAREN expression RIGHT_PAREN
          { $$ = $2; }
          | IDENTIFIER
          { $$ = createVariableNode($1); }
          | INTEGER
          { $$ = createConstantNode($1); }
          ;

%%

int yyerror(const char* s) {
    fprintf(stderr, "语法错误: %s\n", s);
    return 0;
}
```

### ANTLR

ANTLR是现代的语法分析器生成器，支持多种目标语言：

```antlr
grammar SimpleC;

program: (functionDecl | varDecl)+;

functionDecl: type IDENTIFIER '(' paramList? ')' compoundStmt;

paramList: param (',' param)*;

param: type IDENTIFIER;

type: 'int' | 'float' | 'char' | 'void';

compoundStmt: '{' stmt* '}';

stmt: varDecl
    | exprStmt
    | compoundStmt
    | ifStmt
    | whileStmt
    | returnStmt
    ;

exprStmt: expression? ';';

ifStmt: 'if' '(' expression ')' stmt ('else' stmt)?;

whileStmt: 'while' '(' expression ')' stmt;

returnStmt: 'return' expression? ';';

expression: assignment;

assignment: IDENTIFIER '=' assignment
          | logicalOr;

logicalOr: logicalAnd ('||' logicalAnd)*;

logicalAnd: equality ('&&' equality)*;

equality: relational (('==' | '!=') relational)*;

relational: additive (('<' | '>' | '<=' | '>=') additive)*;

additive: multiplicative (('+' | '-') multiplicative)*;

multiplicative: unary (('*' | '/' | '%') unary)*;

unary: ('+' | '-' | '!') unary
     | primary;

primary: IDENTIFIER
       | INTEGER
       | FLOAT
       | STRING
       | '(' expression ')';

// 词法规则
IDENTIFIER: [a-zA-Z_][a-zA-Z0-9_]*;
INTEGER: [0-9]+;
FLOAT: [0-9]+ '.' [0-9]+;
STRING: '"' .*? '"';

WS: [ \t\r\n]+ -> skip;
```

## 性能优化

### 语法分析器优化技巧

1. **消除左递归**：避免递归下降分析器的无限递归
2. **左因子提取**：减少回溯
3. **查找表优化**：快速查找产生式
4. **内存池**：减少AST节点的内存分配开销

```c
// 内存池用于AST节点分配
typedef struct {
    ASTNode* nodes;
    int size;
    int used;
    int capacity;
} ASTNodePool;

ASTNodePool* createASTPool(int initial_capacity) {
    ASTNodePool* pool = malloc(sizeof(ASTNodePool));
    pool->nodes = malloc(initial_capacity * sizeof(ASTNode));
    pool->capacity = initial_capacity;
    pool->used = 0;
    return pool;
}

ASTNode* allocateASTNode(ASTNodePool* pool) {
    if (pool->used >= pool->capacity) {
        pool->capacity *= 2;
        pool->nodes = realloc(pool->nodes,
                             pool->capacity * sizeof(ASTNode));
    }

    ASTNode* node = &pool->nodes[pool->used++];
    memset(node, 0, sizeof(ASTNode));
    return node;
}
```

## 调试和测试

### 语法分析器调试

1. **AST可视化**：打印语法树结构
2. **产生式跟踪**：记录使用的产生式
3. **错误恢复跟踪**：监控错误恢复过程

```c
void printAST(ASTNode* node, int depth) {
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    printf("%s", nodeTypeToString(node->type));
    if (node->value) {
        printf(": %s", node->value);
    }
    printf("\n");

    for (int i = 0; i < node->child_count; i++) {
        printAST(node->children[i], depth + 1);
    }
}
```

### 测试用例设计

```c
// 语法正确性测试
const char* valid_tests[] = {
    "int x = 42;",
    "if (x > 0) { return x; }",
    "while (i < 10) { i++; }",
    "int add(int a, int b) { return a + b; }",
    "x = (a + b) * c / d - e;"
};

// 语法错误测试
const char* invalid_tests[] = {
    "int x = ;",           // 缺少表达式
    "if (x > 0) return;", // 缺少大括号
    "while i < 10 {}",     // 缺少括号
    "int add(int a, b) {}", // 缺少类型
    "x = a + * b;"         // 运算符错误
};
```

## 总结

语法分析是编译器的核心组件，它将线性的标记序列转换为层次化的语法结构。选择合适的分析技术取决于：

- **文法复杂性**：简单文法适合递归下降，复杂文法适合LR分析
- **性能要求**：LR分析器通常更快，但递归下降更易理解和调试
- **错误恢复**：需要良好的错误恢复机制来提供有用的错误信息
- **工具支持**：现代编译器通常使用生成工具来简化开发

理解语法分析的工作原理有助于我们设计更好的编程语言，构建更高效的编译器，并编写更易于解析的程序代码。在下一篇文章中，我们将探讨语义分析，如何验证程序的语义正确性并构建符号表。