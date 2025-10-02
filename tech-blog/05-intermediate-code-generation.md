# 中间代码生成

## 中间代码生成概述

中间代码生成是编译器前端和后端的桥梁，它将经过语义分析验证的抽象语法树（AST）转换为独立于具体目标机器的中间表示（Intermediate Representation, IR）。这种分离使得编译器可以支持多种目标平台，并且便于进行各种代码优化。

### 中间代码的优势

- **平台无关性**：同一中间代码可以生成不同平台的机器码
- **优化友好**：中间代码结构清晰，便于进行各种优化变换
- **模块化设计**：前端和后端解耦，便于维护和扩展
- **调试友好**：中间代码可读性强，便于调试和验证

## 中间代码表示形式

### 三地址码（Three-Address Code）

三地址码是最常用的中间表示形式之一，每条指令最多包含三个操作数：

```
x = y op z
x = op y
goto L
if x goto L
if x relop y goto L
param x
call y, n
return x
```

#### 三地址码示例

**源代码**：
```c
a = b + c * d;
if (a > 10) {
    result = a * 2;
} else {
    result = a / 2;
}
```

**对应的三地址码**：
```
t1 = c * d     // 临时变量存储乘法结果
t2 = b + t1    // 临时变量存储加法结果
a = t2         // 赋值
if a > 10 goto L1  // 条件跳转
goto L2
L1:
t3 = a * 2
result = t3
goto L3
L2:
t4 = a / 2
result = t4
L3:
```

### 静态单赋值形式（SSA）

SSA是一种更高级的中间表示，每个变量只被赋值一次，通过φ函数合并不同路径的值：

```
v1 = 1
v2 = 2
if condition goto L1
v3 = 3
goto L2
L1:
v4 = 4
L2:
v5 = φ(v3, v4)  // φ函数选择来自不同路径的值
```

### 控制流图（CFG）

控制流图是程序控制结构的图形表示，节点代表基本块，边代表控制流：

```
[Entry]
   |
   v
[B1: a = b + c]
   |
   v
[B2: if a > 10]---> [B3: result = a * 2]
   |                  |
   |                  v
   +-----> [B4: result = a / 2]
             |
             v
         [Exit]
```

### 基本块（Basic Block）

基本块是控制流图的基本单位，具有以下特性：
- 只有一个入口点，第一条指令
- 只有一个出口点，最后一条指令（跳转或返回）
- 没有内部跳转或标签

## 中间代码的数据结构

### 指令表示

```c
typedef enum {
    OP_ASSIGN,        // 赋值操作
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, // 算术运算
    OP_NEG,           // 取负
    OP_AND, OP_OR, OP_NOT,  // 逻辑运算
    OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE, // 比较运算
    OP_GOTO,          // 无条件跳转
    OP_IF_GOTO,       // 条件跳转
    OP_CALL,          // 函数调用
    OP_RETURN,        // 函数返回
    OP_PARAM,         // 参数传递
    OP_PHI,           // SSA的φ函数
    OP_LOAD,          // 内存加载
    OP_STORE,         // 内存存储
    OP_ALLOCA         // 栈内存分配
} OpCode;

typedef enum {
    OPERAND_CONSTANT,
    OPERAND_VARIABLE,
    OPERAND_TEMPORARY,
    OPERAND_LABEL,
    OPERAND_FUNCTION
} OperandType;

typedef struct Operand {
    OperandType type;
    union {
        int int_value;
        float float_value;
        char* string_value;
        struct {
            char* name;
            int version;  // SSA版本号
        } var;
        struct {
            char* label;
            int label_id;
        } label;
        struct {
            char* name;
            int param_count;
        } function;
    } value;
} Operand;

typedef struct IRInstruction {
    OpCode op;
    Operand* result;
    Operand* operands[3];  // 最多三个操作数
    int operand_count;
    struct IRInstruction* next;
    struct IRInstruction* prev;
    int line_number;
} IRInstruction;
```

### 基本块表示

```c
typedef struct BasicBlock {
    int id;
    char* name;
    IRInstruction* first_inst;
    IRInstruction* last_inst;
    struct BasicBlock** predecessors;
    int pred_count;
    struct BasicBlock** successors;
    int succ_count;
    struct BasicBlock* immediate_dominator;
    struct BasicBlock* dominator_parent;
    bool visited;
    void* metadata;  // 用于优化和代码生成
} BasicBlock;
```

### 控制流图表示

```c
typedef struct ControlFlowGraph {
    BasicBlock* entry_block;
    BasicBlock* exit_block;
    BasicBlock* blocks;
    int block_count;
    // 寄存器分配信息
    int virtual_register_count;
    // SSA相关
    bool is_ssa_form;
    // 函数信息
    char* function_name;
    int parameter_count;
} ControlFlowGraph;
```

## 中间代码生成算法

### AST到三地址码的转换

```c
typedef struct CodeGenContext {
    ControlFlowGraph* cfg;
    BasicBlock* current_block;
    int temp_count;
    int label_count;
    SymbolTable* symbol_table;
    IRInstruction* current_inst;
} CodeGenContext;

// 创建代码生成上下文
CodeGenContext* createCodeGenContext(SymbolTable* symbol_table) {
    CodeGenContext* ctx = malloc(sizeof(CodeGenContext));
    ctx->cfg = createControlFlowGraph();
    ctx->current_block = createBasicBlock(ctx->cfg, "entry");
    ctx->temp_count = 0;
    ctx->label_count = 0;
    ctx->symbol_table = symbol_table;
    ctx->current_inst = NULL;
    return ctx;
}

// 生成临时变量
Operand* createTempVariable(CodeGenContext* ctx, TypeInfo* type) {
    Operand* temp = malloc(sizeof(Operand));
    temp->type = OPERAND_TEMPORARY;
    temp->value.var.name = malloc(32);
    sprintf(temp->value.var.name, "t%d", ctx->temp_count++);
    temp->value.var.version = 0;
    return temp;
}

// 生成标签
Operand* createLabel(CodeGenContext* ctx) {
    Operand* label = malloc(sizeof(Operand));
    label->type = OPERAND_LABEL;
    label->value.label.label = malloc(32);
    sprintf(label->value.label.label, "L%d", ctx->label_count++);
    label->value.label.label_id = ctx->label_count;
    return label;
}

// 创建指令
IRInstruction* createInstruction(CodeGenContext* ctx, OpCode op,
                               Operand* result, Operand* op1, Operand* op2) {
    IRInstruction* inst = malloc(sizeof(IRInstruction));
    inst->op = op;
    inst->result = result;
    inst->operand_count = 0;

    if (op1 != NULL) {
        inst->operands[inst->operand_count++] = op1;
    }
    if (op2 != NULL) {
        inst->operands[inst->operand_count++] = op2;
    }

    // 添加到当前基本块
    if (ctx->current_block->first_inst == NULL) {
        ctx->current_block->first_inst = inst;
        ctx->current_block->last_inst = inst;
    } else {
        ctx->current_block->last_inst->next = inst;
        inst->prev = ctx->current_block->last_inst;
        ctx->current_block->last_inst = inst;
    }

    return inst;
}
```

### 表达式代码生成

```c
Operand* generateExpression(CodeGenContext* ctx, ASTNode* expr) {
    if (expr == NULL) return NULL;

    switch (expr->type) {
        case NODE_CONSTANT: {
            Operand* constant = malloc(sizeof(Operand));
            constant->type = OPERAND_CONSTANT;

            if (strchr(expr->value, '.') != NULL) {
                constant->value.float_value = atof(expr->value);
            } else {
                constant->value.int_value = atoi(expr->value);
            }

            return constant;
        }

        case NODE_VARIABLE: {
            Operand* var = malloc(sizeof(Operand));
            var->type = OPERAND_VARIABLE;
            var->value.var.name = strdup(expr->value);
            var->value.var.version = 0;
            return var;
        }

        case NODE_BINARY_EXPR: {
            Operand* left = generateExpression(ctx, expr->children[0]);
            Operand* right = generateExpression(ctx, expr->children[1]);
            Operand* result = createTempVariable(ctx, expr->type_info);

            OpCode op = getBinaryOpCode(expr->value);
            createInstruction(ctx, op, result, left, right);

            return result;
        }

        case NODE_UNARY_EXPR: {
            Operand* operand = generateExpression(ctx, expr->children[0]);
            Operand* result = createTempVariable(ctx, expr->type_info);

            OpCode op = getUnaryOpCode(expr->value);
            createInstruction(ctx, op, result, operand, NULL);

            return result;
        }

        case NODE_CALL_EXPR: {
            return generateCallExpression(ctx, expr);
        }

        default:
            return NULL;
    }
}

// 获取二元操作码
OpCode getBinaryOpCode(const char* op_str) {
    if (strcmp(op_str, "+") == 0) return OP_ADD;
    if (strcmp(op_str, "-") == 0) return OP_SUB;
    if (strcmp(op_str, "*") == 0) return OP_MUL;
    if (strcmp(op_str, "/") == 0) return OP_DIV;
    if (strcmp(op_str, "%") == 0) return OP_MOD;
    if (strcmp(op_str, "==") == 0) return OP_EQ;
    if (strcmp(op_str, "!=") == 0) return OP_NE;
    if (strcmp(op_str, "<") == 0) return OP_LT;
    if (strcmp(op_str, ">") == 0) return OP_GT;
    if (strcmp(op_str, "<=") == 0) return OP_LE;
    if (strcmp(op_str, ">=") == 0) return OP_GE;
    return OP_ASSIGN;
}
```

### 语句代码生成

```c
void generateStatement(CodeGenContext* ctx, ASTNode* stmt) {
    if (stmt == NULL) return;

    switch (stmt->type) {
        case NODE_DECL_STMT: {
            generateDeclaration(ctx, stmt);
            break;
        }

        case NODE_ASSIGN_STMT: {
            Operand* left = generateExpression(ctx, stmt->children[0]);
            Operand* right = generateExpression(ctx, stmt->children[1]);
            createInstruction(ctx, OP_ASSIGN, left, right, NULL);
            break;
        }

        case NODE_IF_STMT: {
            generateIfStatement(ctx, stmt);
            break;
        }

        case NODE_WHILE_STMT: {
            generateWhileStatement(ctx, stmt);
            break;
        }

        case NODE_RETURN_STMT: {
            generateReturnStatement(ctx, stmt);
            break;
        }

        case NODE_COMPOUND_STMT: {
            generateCompoundStatement(ctx, stmt);
            break;
        }

        default:
            // 处理其他语句类型
            break;
    }
}
```

### 控制流语句生成

```c
void generateIfStatement(CodeGenContext* ctx, ASTNode* if_stmt) {
    // 生成条件表达式
    Operand* condition = generateExpression(ctx, if_stmt->children[0]);

    // 创建跳转标签
    Operand* false_label = createLabel(ctx);
    Operand* end_label = createLabel(ctx);

    // 生成条件跳转
    createInstruction(ctx, OP_IF_GOTO, NULL, condition, false_label);

    // 生成then分支
    generateStatement(ctx, if_stmt->children[1]);
    createInstruction(ctx, OP_GOTO, NULL, end_label, NULL);

    // 生成false标签和else分支
    BasicBlock* false_block = createBasicBlock(ctx->cfg, false_label->value.label.label);
    ctx->current_block = false_block;
    if (if_stmt->child_count > 2) {
        generateStatement(ctx, if_stmt->children[2]);
    }

    // 生成结束标签
    BasicBlock* end_block = createBasicBlock(ctx->cfg, end_label->value.label.label);
    ctx->current_block = end_block;
}

void generateWhileStatement(CodeGenContext* ctx, ASTNode* while_stmt) {
    // 创建循环标签
    Operand* condition_label = createLabel(ctx);
    Operand* end_label = createLabel(ctx);

    // 生成条件检查标签
    BasicBlock* condition_block = createBasicBlock(ctx->cfg, condition_label->value.label.label);
    ctx->current_block = condition_block;

    // 生成条件表达式
    Operand* condition = generateExpression(ctx, while_stmt->children[0]);

    // 生成条件跳转
    createInstruction(ctx, OP_IF_GOTO, NULL, condition, end_label);

    // 生成循环体
    BasicBlock* body_block = createBasicBlock(ctx->cfg, "loop_body");
    ctx->current_block = body_block;
    generateStatement(ctx, while_stmt->children[1]);

    // 跳回条件检查
    createInstruction(ctx, OP_GOTO, NULL, condition_label, NULL);

    // 生成结束标签
    BasicBlock* end_block = createBasicBlock(ctx->cfg, end_label->value.label.label);
    ctx->current_block = end_block;
}
```

### 函数调用生成

```c
Operand* generateCallExpression(CodeGenContext* ctx, ASTNode* call_expr) {
    // 生成参数
    for (int i = 1; i < call_expr->child_count; i++) {
        Operand* arg = generateExpression(ctx, call_expr->children[i]);
        createInstruction(ctx, OP_PARAM, NULL, arg, NULL);
    }

    // 创建函数调用操作数
    Operand* func = malloc(sizeof(Operand));
    func->type = OPERAND_FUNCTION;
    func->value.function.name = call_expr->children[0]->value;
    func->value.function.param_count = call_expr->child_count - 1;

    // 创建返回值临时变量
    Operand* result = createTempVariable(ctx, call_expr->type_info);

    // 生成调用指令
    createInstruction(ctx, OP_CALL, result, func, NULL);

    return result;
}
```

## 控制流图构建

### 基本块识别

```c
void identifyBasicBlocks(CodeGenContext* ctx) {
    BasicBlock* current_block = createBasicBlock(ctx->cfg, "entry");
    IRInstruction* inst = ctx->cfg->entry_block->first_inst;

    while (inst != NULL) {
        // 如果是标签或跳转指令，需要创建新基本块
        if (isLeaderInstruction(inst)) {
            if (current_block->first_inst != NULL) {
                current_block = createBasicBlock(ctx->cfg, "block");
            }
        }

        // 将指令添加到当前基本块
        if (current_block->first_inst == NULL) {
            current_block->first_inst = inst;
            current_block->last_inst = inst;
        } else {
            current_block->last_inst->next = inst;
            inst->prev = current_block->last_inst;
            current_block->last_inst = inst;
        }

        // 如果是跳转或返回指令，结束当前基本块
        if (isTerminatorInstruction(inst)) {
            current_block = createBasicBlock(ctx->cfg, "block");
        }

        inst = inst->next;
    }
}

bool isLeaderInstruction(IRInstruction* inst) {
    return inst->op == OP_GOTO || inst->op == OP_IF_GOTO ||
           inst->op == OP_RETURN || inst->result != NULL &&
           inst->result->type == OPERAND_LABEL;
}

bool isTerminatorInstruction(IRInstruction* inst) {
    return inst->op == OP_GOTO || inst->op == OP_IF_GOTO ||
           inst->op == OP_RETURN;
}
```

### 控制流边构建

```c
void buildControlFlowEdges(ControlFlowGraph* cfg) {
    // 遍历所有基本块
    for (int i = 0; i < cfg->block_count; i++) {
        BasicBlock* block = cfg->blocks[i];
        IRInstruction* last_inst = block->last_inst;

        if (last_inst == NULL) continue;

        switch (last_inst->op) {
            case OP_GOTO: {
                // 无条件跳转，添加到目标块的边
                BasicBlock* target = findBlockByLabel(cfg, last_inst->operands[0]);
                addSuccessor(block, target);
                break;
            }

            case OP_IF_GOTO: {
                // 条件跳转，添加到true和false目标块的边
                BasicBlock* true_target = findBlockByLabel(cfg, last_inst->operands[0]);
                BasicBlock* false_target = cfg->blocks[i + 1]; // 下一块
                addSuccessor(block, true_target);
                addSuccessor(block, false_target);
                break;
            }

            case OP_RETURN: {
                // 返回指令，没有后继
                break;
            }

            default: {
                // 普通指令，默认流向下一块
                if (i < cfg->block_count - 1) {
                    addSuccessor(block, cfg->blocks[i + 1]);
                }
                break;
            }
        }
    }
}

void addSuccessor(BasicBlock* from, BasicBlock* to) {
    // 添加到from的后继列表
    from->successors = realloc(from->successors,
                             (from->succ_count + 1) * sizeof(BasicBlock*));
    from->successors[from->succ_count++] = to;

    // 添加到to的前驱列表
    to->predecessors = realloc(to->predecessors,
                             (to->pred_count + 1) * sizeof(BasicBlock*));
    to->predecessors[to->pred_count++] = from;
}
```

## SSA转换

### SSA转换算法

```c
void convertToSSA(ControlFlowGraph* cfg) {
    // 计算支配边界
    computeDominanceFrontiers(cfg);

    // 插入φ函数
    insertPhiFunctions(cfg);

    // 重命名变量
    renameVariables(cfg);
}

void computeDominanceFrontiers(ControlFlowGraph* cfg) {
    for (int i = 0; i < cfg->block_count; i++) {
        BasicBlock* block = cfg->blocks[i];

        // 如果块的严格前驱数量 > 1
        if (block->pred_count > 1) {
            for (int j = 0; j < block->pred_count; j++) {
                BasicBlock* runner = block->predecessors[j];

                while (runner != block->immediate_dominator) {
                    // 将block添加到runner的支配边界
                    addToDominanceFrontier(runner, block);
                    runner = runner->immediate_dominator;
                }
            }
        }
    }
}

void insertPhiFunctions(ControlFlowGraph* cfg) {
    // 遍历所有全局变量
    for (Variable* var = cfg->global_variables; var != NULL; var = var->next) {
        // 遍历所有基本块
        for (int i = 0; i < cfg->block_count; i++) {
            BasicBlock* block = cfg->blocks[i];

            // 检查是否需要在块首插入φ函数
            if (needsPhiFunction(block, var)) {
                IRInstruction* phi = createPhiInstruction(block, var);
                insertInstructionAtBeginning(block, phi);
            }
        }
    }
}
```

### 变量重命名

```c
void renameVariables(ControlFlowGraph* cfg) {
    // 初始化计数器栈
    Stack* counter_stacks = createStack(cfg->variable_count);
    for (int i = 0; i < cfg->variable_count; i++) {
        pushStack(counter_stacks[i], 0);  // 初始版本0
    }

    // 从入口块开始重命名
    renameBlock(cfg->entry_block, counter_stacks);
}

void renameBlock(BasicBlock* block, Stack* counter_stacks) {
    // 重命名块内的所有指令
    for (IRInstruction* inst = block->first_inst; inst != NULL; inst = inst->next) {
        renameInstruction(inst, counter_stacks);
    }

    // 重命名后继块的φ函数参数
    for (int i = 0; i < block->succ_count; i++) {
        BasicBlock* succ = block->successors[i];
        for (IRInstruction* phi = succ->first_inst; phi != NULL && phi->op == OP_PHI; phi = phi->next) {
            // 找到对应的参数并重命名
            renamePhiOperand(phi, block, counter_stacks);
        }
    }

    // 递归处理支配的子块
    for (BasicBlock* child = block->dominator_child; child != NULL; child = child->dominator_sibling) {
        renameBlock(child, counter_stacks);
    }

    // 恢复计数器栈
    for (IRInstruction* inst = block->first_inst; inst != NULL; inst = inst->next) {
        if (inst->result != NULL && inst->result->type == OPERAND_VARIABLE) {
            popStack(counter_stacks[inst->result->value.var.index]);
        }
    }
}
```

## 优化准备

### 基本块优化

```c
void optimizeBasicBlock(BasicBlock* block) {
    // 常量折叠
    constantFolding(block);

    // 死代码消除
    deadCodeElimination(block);

    // 代数简化
    algebraicSimplification(block);

    // 复制传播
    copyPropagation(block);
}

void constantFolding(BasicBlock* block) {
    for (IRInstruction* inst = block->first_inst; inst != NULL; inst = inst->next) {
        if (isConstantExpression(inst)) {
            // 计算常量表达式的值
            int result = evaluateConstantExpression(inst);

            // 替换为常量加载
            inst->op = OP_ASSIGN;
            inst->operands[0] = createConstantOperand(result);
            inst->operand_count = 1;
        }
    }
}
```

## 实际案例分析

### 示例程序转换

**源代码**：
```c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}
```

**AST结构**：
```
FunctionDecl: factorial
├── Parameter: n (int)
└── CompoundStmt
    └── IfStmt
        ├── Condition: n <= 1
        ├── Then: ReturnStmt with 1
        └── Else: ReturnStmt with n * factorial(n - 1)
```

**生成的三地址码**：
```
factorial:
    if n <= 1 goto L1
    goto L2
L1:
    return 1
L2:
    t1 = n - 1
    t2 = call factorial, t1
    t3 = n * t2
    return t3
```

**SSA形式**：
```
factorial:
    if n1 <= 1 goto L1
    goto L2
L1:
    return 1
L2:
    t1 = n1 - 1
    t2 = call factorial, t1
    t3 = n1 * t2
    return t3
```

### 控制流图

```
[Entry]
   |
   v
[B1: if n1 <= 1]
   / \
  /   \
[B2] [B3: return 1]
  |      |
  |      v
  +---> [B4: t1 = n1 - 1]
         |
         v
[B5: t2 = call factorial, t1]
         |
         v
[B6: t3 = n1 * t2]
         |
         v
     [Exit]
```

## 高级话题

### 内存操作

```c
// 指针解引用
Operand* generateDereference(CodeGenContext* ctx, Operand* ptr) {
    Operand* result = createTempVariable(ctx, getPointerTypeBaseType(ptr->type));
    createInstruction(ctx, OP_LOAD, result, ptr, NULL);
    return result;
}

// 指针赋值
void generateStore(CodeGenContext* ctx, Operand* ptr, Operand* value) {
    createInstruction(ctx, OP_STORE, NULL, ptr, value);
}
```

### 异常处理

```c
void generateExceptionHandling(CodeGenContext* ctx, ASTNode* try_catch) {
    // 生成try块
    BasicBlock* try_block = createBasicBlock(ctx->cfg, "try_block");
    BasicBlock* catch_block = createBasicBlock(ctx->cfg, "catch_block");
    BasicBlock* finally_block = createBasicBlock(ctx->cfg, "finally_block");

    // 设置异常处理信息
    ctx->current_block = try_block;
    generateStatement(ctx, try_catch->children[0]);

    // 生成catch块
    ctx->current_block = catch_block;
    generateStatement(ctx, try_catch->children[1]);

    // 生成finally块
    ctx->current_block = finally_block;
    generateStatement(ctx, try_catch->children[2]);
}
```

## 总结

中间代码生成是编译器设计的核心环节，它将高级语言的结构转换为适合优化和目标代码生成的中间表示。一个良好的中间代码生成系统应该具备：

1. **平台无关性**：能够支持多种目标平台
2. **优化友好**：提供丰富的信息支持各种优化
3. **类型安全**：保留足够的类型信息
4. **控制流清晰**：准确表示程序的控制结构
5. **可扩展性**：便于添加新的语言特性和优化

理解中间代码生成的原理有助于我们构建更高效的编译器，支持更复杂的语言特性，并实现更高级的优化技术。在下一篇文章中，我们将探讨代码优化技术，如何进一步改进中间代码的性能和质量。