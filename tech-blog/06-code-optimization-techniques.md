# 代码优化技术

## 代码优化概述

代码优化是编译器后端的重要组成部分，它在保证程序语义正确的前提下，通过各种变换技术改进代码的性能、减少内存占用、降低功耗等。优化可以在多个层次进行：源代码层、中间代码层、目标代码层，甚至运行时层。

### 优化的目标

- **性能提升**：减少执行时间，提高程序运行速度
- **空间优化**：减少代码大小和数据内存占用
- **功耗优化**：降低能耗，适用于移动和嵌入式设备
- **可预测性**：改善缓存行为，减少分支预测失败
- **并行化**：利用多核和SIMD指令集

### 优化的分类

#### 按优化范围分类
- **局部优化**：在基本块内部进行的优化
- **全局优化**：在整个函数范围内进行的优化
- **过程间优化**：跨函数边界的优化
- **链接时优化**：在链接阶段进行的优化

#### 按优化时机分类
- **编译时优化**：在编译阶段进行的优化
- **链接时优化**：在链接阶段进行的优化
- **运行时优化**：在程序运行时进行的优化（JIT编译器）

## 局部优化技术

### 常量折叠（Constant Folding）

常量折叠是在编译时计算常量表达式的值，避免运行时计算。

```c
// 原始代码
int x = 2 + 3 * 4;
int y = 10 / 2;
bool z = (5 > 3) && (2 < 4);

// 优化后代码
int x = 14;
int y = 5;
bool z = true;
```

#### 实现算法

```c
bool tryConstantFolding(IRInstruction* inst) {
    // 检查是否为二元运算
    if (inst->op >= OP_ADD && inst->op <= OP_MOD) {
        Operand* left = inst->operands[0];
        Operand* right = inst->operands[1];

        // 检查两个操作数是否都是常量
        if (left->type == OPERAND_CONSTANT && right->type == OPERAND_CONSTANT) {
            int result = evaluateBinaryOperation(inst->op, left, right);

            // 替换为常量赋值
            inst->op = OP_ASSIGN;
            inst->operands[0] = createConstantOperand(result);
            inst->operand_count = 1;
            return true;
        }
    }
    return false;
}

int evaluateBinaryOperation(OpCode op, Operand* left, Operand* right) {
    int a = left->value.int_value;
    int b = right->value.int_value;

    switch (op) {
        case OP_ADD: return a + b;
        case OP_SUB: return a - b;
        case OP_MUL: return a * b;
        case OP_DIV: return b != 0 ? a / b : 0;  // 处理除零
        case OP_MOD: return b != 0 ? a % b : 0;
        default: return 0;
    }
}
```

### 常量传播（Constant Propagation）

常量传播将已知的常量值传播到使用该变量的位置。

```c
// 原始代码
int x = 10;
int y = x + 5;
int z = y * 2;

// 优化过程
// 第一步：传播x的值
int y = 10 + 5;  // x替换为10
int z = y * 2;

// 第二步：计算y的值并传播
int y = 15;
int z = 15 * 2;

// 最终结果
int y = 15;
int z = 30;
```

#### 实现算法

```c
void constantPropagation(BasicBlock* block) {
    // 维护变量的常量值映射
    HashMap* constant_values = createHashMap();

    for (IRInstruction* inst = block->first_inst; inst != NULL; inst = inst->next) {
        // 检查操作数是否为已知常量
        for (int i = 0; i < inst->operand_count; i++) {
            Operand* operand = inst->operands[i];
            if (operand->type == OPERAND_VARIABLE) {
                int* const_value = hashMapGet(constant_values, operand->value.var.name);
                if (const_value != NULL) {
                    // 替换为常量
                    inst->operands[i] = createConstantOperand(*const_value);
                }
            }
        }

        // 尝试常量折叠
        tryConstantFolding(inst);

        // 更新常量值映射
        if (inst->op == OP_ASSIGN && inst->operands[0]->type == OPERAND_CONSTANT) {
            hashMapPut(constant_values, inst->result->value.var.name,
                      &(inst->operands[0]->value.int_value));
        } else if (inst->result != NULL) {
            // 变量被重新赋值，从映射中移除
            hashMapRemove(constant_values, inst->result->value.var.name);
        }
    }

    freeHashMap(constant_values);
}
```

### 死代码消除（Dead Code Elimination）

死代码消除移除不会影响程序结果的代码。

```c
// 原始代码
if (false) {
    x = 10;  // 死代码，永远不会执行
}
return x;

// 优化后代码
return x;  // 移除了if语句和死代码
```

#### 实现算法

```c
void deadCodeElimination(ControlFlowGraph* cfg) {
    // 标记可达的基本块
    markReachableBlocks(cfg->entry_block);

    // 移除不可达的基本块
    removeUnreachableBlocks(cfg);

    // 在每个基本块中移除死指令
    for (int i = 0; i < cfg->block_count; i++) {
        BasicBlock* block = cfg->blocks[i];
        if (block->visited) {
            removeDeadInstructions(block);
        }
    }
}

void markReachableBlocks(BasicBlock* block) {
    if (block->visited) return;

    block->visited = true;

    // 标记所有后继块
    for (int i = 0; i < block->succ_count; i++) {
        markReachableBlocks(block->successors[i]);
    }
}

void removeDeadInstructions(BasicBlock* block) {
    // 第一次遍历：标记有用的指令
    HashMap* useful_instructions = createHashMap();

    // 标记所有有副作用的指令（函数调用、内存操作等）
    for (IRInstruction* inst = block->first_inst; inst != NULL; inst = inst->next) {
        if (hasSideEffects(inst)) {
            hashMapPut(useful_instructions, inst, (void*)1);
            // 标记操作数定义
            markOperandDefinitions(inst, useful_instructions);
        }
    }

    // 第二次遍历：移除无用的指令
    IRInstruction* prev = NULL;
    IRInstruction* current = block->first_inst;

    while (current != NULL) {
        IRInstruction* next = current->next;

        if (hashMapGet(useful_instructions, current) == NULL) {
            // 移除无用的指令
            if (prev == NULL) {
                block->first_inst = next;
            } else {
                prev->next = next;
            }
            if (next != NULL) {
                next->prev = prev;
            }
            freeInstruction(current);
        } else {
            prev = current;
        }
        current = next;
    }

    freeHashMap(useful_instructions);
}

bool hasSideEffects(IRInstruction* inst) {
    return inst->op == OP_CALL || inst->op == OP_STORE ||
           inst->op == OP_RETURN || inst->op == OP_PARAM;
}
```

### 代数简化（Algebraic Simplification）

代数简化使用数学恒等式简化表达式。

```c
// 原始代码
x = y + 0;      // 简化为 x = y
x = y * 1;      // 简化为 x = y
x = y * 0;      // 简化为 x = 0
x = y / 1;      // 简化为 x = y
x = x * x;      // 简化为 x = x * x (或 pow(x, 2))
```

#### 实现算法

```c
bool algebraicSimplification(IRInstruction* inst) {
    switch (inst->op) {
        case OP_ADD:
            return simplifyAddition(inst);
        case OP_SUB:
            return simplifySubtraction(inst);
        case OP_MUL:
            return simplifyMultiplication(inst);
        case OP_DIV:
            return simplifyDivision(inst);
        default:
            return false;
    }
}

bool simplifyMultiplication(IRInstruction* inst) {
    Operand* left = inst->operands[0];
    Operand* right = inst->operands[1];

    // x * 0 = 0
    if (isConstantZero(left) || isConstantZero(right)) {
        inst->op = OP_ASSIGN;
        inst->operands[0] = createConstantOperand(0);
        inst->operand_count = 1;
        return true;
    }

    // x * 1 = x
    if (isConstantOne(left)) {
        inst->op = OP_ASSIGN;
        inst->operands[0] = right;
        inst->operand_count = 1;
        return true;
    }
    if (isConstantOne(right)) {
        inst->op = OP_ASSIGN;
        inst->operands[0] = left;
        inst->operand_count = 1;
        return true;
    }

    return false;
}

bool simplifyAddition(IRInstruction* inst) {
    Operand* left = inst->operands[0];
    Operand* right = inst->operands[1];

    // x + 0 = x
    if (isConstantZero(left)) {
        inst->op = OP_ASSIGN;
        inst->operands[0] = right;
        inst->operand_count = 1;
        return true;
    }
    if (isConstantZero(right)) {
        inst->op = OP_ASSIGN;
        inst->operands[0] = left;
        inst->operand_count = 1;
        return true;
    }

    return false;
}
```

## 全局优化技术

### 公共子表达式消除（Common Subexpression Elimination）

公共子表达式消除识别和消除重复计算的子表达式。

```c
// 原始代码
a = (b + c) * d;
e = (b + c) / f;

// 优化后代码
t1 = b + c;  // 公共子表达式
a = t1 * d;
e = t1 / f;
```

#### 实现算法

```c
void commonSubexpressionElimination(ControlFlowGraph* cfg) {
    // 计算每个基本块的可用表达式
    computeAvailableExpressions(cfg);

    // 在每个基本块中消除公共子表达式
    for (int i = 0; i < cfg->block_count; i++) {
        BasicBlock* block = cfg->blocks[i];
        eliminateCommonSubexpressionsInBlock(block);
    }
}

void eliminateCommonSubexpressionsInBlock(BasicBlock* block) {
    HashMap* expression_to_temp = createHashMap();

    for (IRInstruction* inst = block->first_inst; inst != NULL; inst = inst->next) {
        // 检查是否为可消除的表达式
        if (isComputableExpression(inst)) {
            char* expression_key = generateExpressionKey(inst);

            // 检查是否已经计算过
            Operand* existing_temp = hashMapGet(expression_to_temp, expression_key);
            if (existing_temp != NULL) {
                // 替换为已计算的结果
                inst->op = OP_ASSIGN;
                inst->operands[0] = existing_temp;
                inst->operand_count = 1;
            } else {
                // 创建新的临时变量存储结果
                Operand* temp = createTempVariable(block->function, inst->result->type);
                hashMapPut(expression_to_temp, expression_key, temp);

                // 更新指令结果为临时变量
                Operand* old_result = inst->result;
                inst->result = temp;

                // 添加赋值指令
                IRInstruction* assign = createInstruction(OP_ASSIGN, old_result, temp, NULL);
                insertInstructionAfter(inst, assign);
            }
        }
    }

    freeHashMap(expression_to_temp);
}
```

### 复制传播（Copy Propagation）

复制传播将赋值传播到使用该变量的位置，减少不必要的赋值操作。

```c
// 原始代码
x = y;
z = x + 1;

// 优化后代码
z = y + 1;
```

#### 实现算法

```c
void copyPropagation(ControlFlowGraph* cfg) {
    for (int i = 0; i < cfg->block_count; i++) {
        BasicBlock* block = cfg->blocks[i];
        copyPropagationInBlock(block);
    }
}

void copyPropagationInBlock(BasicBlock* block) {
    HashMap* copy_map = createHashMap();  // 变量到其复制源的映射

    for (IRInstruction* inst = block->first_inst; inst != NULL; inst = inst->next) {
        // 传播操作数
        for (int i = 0; i < inst->operand_count; i++) {
            Operand* operand = inst->operands[i];
            if (operand->type == OPERAND_VARIABLE) {
                Operand* copy_source = hashMapGet(copy_map, operand->value.var.name);
                if (copy_source != NULL) {
                    inst->operands[i] = copy_source;
                }
            }
        }

        // 更新复制映射
        if (inst->op == OP_ASSIGN && inst->operands[0]->type == OPERAND_VARIABLE) {
            // x = y 的形式
            hashMapPut(copy_map, inst->result->value.var.name, inst->operands[0]);
        } else if (inst->result != NULL) {
            // 变量被重新定义，移除其复制信息
            hashMapRemove(copy_map, inst->result->value.var.name);
        }
    }

    freeHashMap(copy_map);
}
```

### 循环优化

循环是程序中性能关键的部分，循环优化可以显著提升程序性能。

#### 循环不变代码外提（Loop-Invariant Code Motion）

将循环中不变的代码移到循环外面。

```c
// 原始代码
for (i = 0; i < n; i++) {
    a[i] = b * c + d;
}

// 优化后代码
t1 = b * c + d;  // 循环不变量
for (i = 0; i < n; i++) {
    a[i] = t1;
}
```

#### 实现算法

```c
void loopInvariantCodeMotion(Loop* loop) {
    // 识别循环不变指令
    ArrayList* invariant_instructions = identifyLoopInvariantInstructions(loop);

    // 将不变指令移到循环前置块
    BasicBlock* preheader = createPreheader(loop);
    for (int i = 0; i < arrayListSize(invariant_instructions); i++) {
        IRInstruction* inst = arrayListGet(invariant_instructions, i);
        moveInstructionToBlock(inst, preheader);
    }

    freeArrayList(invariant_instructions);
}

bool isLoopInvariantInstruction(IRInstruction* inst, Loop* loop) {
    // 检查所有操作数是否在循环外定义或为常量
    for (int i = 0; i < inst->operand_count; i++) {
        Operand* operand = inst->operands[i];
        if (operand->type == OPERAND_VARIABLE) {
            if (isDefinedInLoop(operand, loop)) {
                return false;
            }
        }
    }
    return true;
}
```

#### 归纳变量优化（Induction Variable Optimization）

识别和优化循环中的归纳变量。

```c
// 原始代码
for (i = 0; i < n; i++) {
    a[i] = i * 4;
}

// 优化后代码
for (i = 0, t = 0; i < n; i++, t += 4) {
    a[i] = t;
}
```

#### 强度削减（Strength Reduction）

用更高效的运算替换昂贵的运算。

```c
// 原始代码
for (i = 0; i < n; i++) {
    a[i] = i * 4;  // 乘法
}

// 优化后代码
for (i = 0, t = 0; i < n; i++, t += 4) {
    a[i] = t;      // 加法
}
```

## 数据流分析

数据流分析是全局优化的基础，它分析程序中数据如何流动。

### 活性分析（Liveness Analysis）

活性分析确定变量在程序点是否"活"着（即在未来可能被使用）。

```c
void livenessAnalysis(ControlFlowGraph* cfg) {
    bool changed;
    do {
        changed = false;

        // 反向遍历所有基本块
        for (int i = cfg->block_count - 1; i >= 0; i--) {
            BasicBlock* block = cfg->blocks[i];

            // 计算block的out集合
            HashSet* new_out = createHashSet();
            for (int j = 0; j < block->succ_count; j++) {
                BasicBlock* succ = block->successors[j];
                hashSetUnion(new_out, succ->live_in);
            }

            // 计算block的in集合
            HashSet* new_in = hashSetCopy(new_out);

            // 移除block中定义的变量
            for (IRInstruction* inst = block->first_inst; inst != NULL; inst = inst->next) {
                if (inst->result != NULL) {
                    hashSetRemove(new_in, inst->result->value.var.name);
                }
            }

            // 添加block中使用的变量
            for (IRInstruction* inst = block->first_inst; inst != NULL; inst = inst->next) {
                for (int j = 0; j < inst->operand_count; j++) {
                    Operand* operand = inst->operands[j];
                    if (operand->type == OPERAND_VARIABLE) {
                        hashSetAdd(new_in, operand->value.var.name);
                    }
                }
            }

            // 检查是否有变化
            if (!hashSetEquals(block->live_out, new_out) ||
                !hashSetEquals(block->live_in, new_in)) {
                hashSetFree(block->live_out);
                hashSetFree(block->live_in);
                block->live_out = new_out;
                block->live_in = new_in;
                changed = true;
            } else {
                hashSetFree(new_out);
                hashSetFree(new_in);
            }
        }
    } while (changed);
}
```

### 可用表达式分析（Available Expressions Analysis）

可用表达式分析确定哪些表达式在每个程序点已经计算过且结果可用。

```c
void availableExpressionsAnalysis(ControlFlowGraph* cfg) {
    // 初始化
    for (int i = 0; i < cfg->block_count; i++) {
        BasicBlock* block = cfg->blocks[i];
        block->avail_in = createHashSet();
        block->avail_out = createHashSet();
        block->gen = createHashSet();
        block->kill = createHashSet();
        computeGenKillSets(block);
    }

    // 入口块的in集合为空
    hashSetClear(cfg->entry_block->avail_in);

    // 迭代求解
    bool changed;
    do {
        changed = false;

        for (int i = 0; i < cfg->block_count; i++) {
            BasicBlock* block = cfg->blocks[i];
            if (block == cfg->entry_block) continue;

            // 计算新的in集合
            HashSet* new_in = createHashSet();
            for (int j = 0; j < block->pred_count; j++) {
                BasicBlock* pred = block->predecessors[j];
                hashSetIntersect(new_in, pred->avail_out);
            }

            // 计算新的out集合
            HashSet* new_out = hashSetCopy(new_in);
            hashSetDifference(new_out, block->kill);
            hashSetUnion(new_out, block->gen);

            // 检查变化
            if (!hashSetEquals(block->avail_in, new_in) ||
                !hashSetEquals(block->avail_out, new_out)) {
                hashSetFree(block->avail_in);
                hashSetFree(block->avail_out);
                block->avail_in = new_in;
                block->avail_out = new_out;
                changed = true;
            } else {
                hashSetFree(new_in);
                hashSetFree(new_out);
            }
        }
    } while (changed);
}
```

## 高级优化技术

### 函数内联（Function Inlining）

函数内联将函数调用替换为函数体，消除函数调用的开销。

```c
// 原始代码
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = add(3, 4);
    return x;
}

// 优化后代码
int main() {
    int x = 3 + 4;  // 内联后的函数体
    return x;
}
```

#### 实现算法

```c
void functionInlining(ControlFlowGraph* cfg) {
    for (int i = 0; i < cfg->block_count; i++) {
        BasicBlock* block = cfg->blocks[i];
        IRInstruction* inst = block->first_inst;

        while (inst != NULL) {
            IRInstruction* next = inst->next;

            if (inst->op == OP_CALL && shouldInline(inst)) {
                inlineFunctionCall(block, inst);
            }
            inst = next;
        }
    }
}

bool shouldInline(IRInstruction* call_inst) {
    Function* callee = getFunction(call_inst->operands[0]->value.function.name);

    // 检查函数大小
    if (callee->instruction_count > INLINE_SIZE_THRESHOLD) {
        return false;
    }

    // 检查递归调用
    if (strcmp(callee->name, currentFunction->name) == 0) {
        return false;
    }

    // 检查调用频率
    if (callee->call_count < INLINE_FREQUENCY_THRESHOLD) {
        return false;
    }

    return true;
}
```

### 尾调用优化（Tail Call Optimization）

尾调用优化将尾递归转换为循环，避免栈溢出。

```c
// 原始代码（尾递归）
int factorial(int n, int acc) {
    if (n <= 1) {
        return acc;
    } else {
        return factorial(n - 1, n * acc);
    }
}

// 优化后代码（循环）
int factorial(int n, int acc) {
    while (n > 1) {
        acc = n * acc;
        n = n - 1;
    }
    return acc;
}
```

### 向量化（Vectorization）

向量化利用SIMD指令集同时处理多个数据元素。

```c
// 原始代码
for (i = 0; i < n; i++) {
    c[i] = a[i] + b[i];
}

// 向量化后代码（假设4路向量）
for (i = 0; i < n - 3; i += 4) {
    vec4 va = load(&a[i]);
    vec4 vb = load(&b[i]);
    vec4 vc = va + vb;
    store(&c[i], vc);
}

// 处理剩余元素
for (; i < n; i++) {
    c[i] = a[i] + b[i];
}
```

## 优化框架

### 优化管道设计

```c
typedef struct OptimizationPass {
    const char* name;
    bool (*run)(ControlFlowGraph* cfg);
    bool is_enabled;
    int priority;
} OptimizationPass;

typedef struct OptimizationPipeline {
    OptimizationPass* passes;
    int pass_count;
    bool verbose;
} OptimizationPipeline;

// 创建优化管道
OptimizationPipeline* createOptimizationPipeline() {
    OptimizationPipeline* pipeline = malloc(sizeof(OptimizationPipeline));
    pipeline->pass_count = 0;
    pipeline->verbose = false;

    // 添加优化passes
    addOptimizationPass(pipeline, "constant_folding", constantFoldingPass, true, 1);
    addOptimizationPass(pipeline, "dead_code_elimination", deadCodeEliminationPass, true, 2);
    addOptimizationPass(pipeline, "copy_propagation", copyPropagationPass, true, 3);
    addOptimizationPass(pipeline, "common_subexpression_elimination", csePass, true, 4);
    addOptimizationPass(pipeline, "loop_optimization", loopOptimizationPass, true, 5);
    addOptimizationPass(pipeline, "function_inlining", inliningPass, true, 6);

    return pipeline;
}

// 运行优化管道
bool runOptimizationPipeline(OptimizationPipeline* pipeline, ControlFlowGraph* cfg) {
    for (int i = 0; i < pipeline->pass_count; i++) {
        OptimizationPass* pass = &pipeline->passes[i];
        if (pass->is_enabled) {
            if (pipeline->verbose) {
                printf("Running optimization pass: %s\n", pass->name);
            }

            if (!pass->run(cfg)) {
                fprintf(stderr, "Optimization pass %s failed\n", pass->name);
                return false;
            }
        }
    }
    return true;
}
```

### 优化效果评估

```c
typedef struct OptimizationMetrics {
    int original_instruction_count;
    int optimized_instruction_count;
    int basic_block_count;
    int eliminated_instructions;
    double optimization_time;
    int memory_usage;
} OptimizationMetrics;

OptimizationMetrics evaluateOptimization(ControlFlowGraph* original_cfg,
                                       ControlFlowGraph* optimized_cfg) {
    OptimizationMetrics metrics;

    metrics.original_instruction_count = countInstructions(original_cfg);
    metrics.optimized_instruction_count = countInstructions(optimized_cfg);
    metrics.basic_block_count = optimized_cfg->block_count;
    metrics.eliminated_instructions = metrics.original_instruction_count -
                                     metrics.optimized_instruction_count;

    double reduction_percentage = (double)metrics.eliminated_instructions /
                                metrics.original_instruction_count * 100.0;

    printf("Optimization Results:\n");
    printf("  Original instructions: %d\n", metrics.original_instruction_count);
    printf("  Optimized instructions: %d\n", metrics.optimized_instruction_count);
    printf("  Eliminated instructions: %d (%.1f%%)\n",
           metrics.eliminated_instructions, reduction_percentage);
    printf("  Basic blocks: %d\n", metrics.basic_block_count);

    return metrics;
}
```

## 实际案例分析

### 优化前后对比

**原始代码**：
```c
int sum_array(int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum = sum + arr[i];
    }
    return sum;
}
```

**中间代码（优化前）**：
```
t1 = 0
sum = t1
i = 0
goto L1
L1:
if i < n goto L2
goto L3
L2:
t2 = i * 4
t3 = arr + t2
t4 = *t3
t5 = sum + t4
sum = t5
t6 = i + 1
i = t6
goto L1
L3:
return sum
```

**中间代码（优化后）**：
```
sum = 0
i = 0
goto L1
L1:
if i >= n goto L3
t1 = arr[i]
sum = sum + t1
i = i + 1
goto L1
L3:
return sum
```

**优化效果**：
- 指令数量：从12条减少到9条
- 消除了临时变量t2, t3, t4, t5, t6
- 简化了数组访问（使用数组语法而不是指针算术）
- 减少了寄存器压力

## 总结

代码优化是编译器的核心功能之一，它能显著提升程序的性能和质量。现代编译器通常实现多层次的优化策略：

1. **局部优化**：在基本块级别进行快速优化
2. **全局优化**：利用数据流分析进行跨基本块优化
3. **循环优化**：针对程序热点进行专门优化
4. **高级优化**：如函数内联、向量化等复杂变换

一个好的优化系统应该具备：
- **正确性**：保证优化不改变程序语义
- **效率**：优化过程本身不应太耗时
- **可扩展性**：便于添加新的优化算法
- **可配置性**：允许用户选择优化级别和策略
- **可调试性**：提供优化过程的可视化和调试信息

理解代码优化技术不仅有助于构建更好的编译器，也能帮助我们编写更高效的程序，并深入理解计算机系统的性能特征。在下一篇文章中，我们将探讨目标代码生成，如何将优化后的中间代码转换为特定平台的机器码。