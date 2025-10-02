# 运行时系统与内存管理

## 运行时系统概述

运行时系统（Runtime System）是支持程序执行的一组库和服务，它为编译生成的代码提供执行环境。运行时系统负责内存管理、异常处理、垃圾回收、线程管理、动态类型检查等任务，是现代编程语言不可或缺的组成部分。

### 运行时系统的组成

- **内存管理器**：负责内存分配、回收和管理
- **垃圾收集器**：自动回收不再使用的内存
- **异常处理系统**：提供结构化的错误处理机制
- **类型系统运行时支持**：动态类型检查和转换
- **线程和同步支持**：并发编程的支持
- **标准库运行时**：基础数据结构和算法的实现
- **调试和监控**：性能分析、内存泄漏检测等

### 编译时 vs 运行时

**编译时（Compile Time）**：
- 类型检查
- 语法分析
- 代码优化
- 静态链接

**运行时（Runtime）**：
- 内存分配和释放
- 动态类型检查
- 异常处理
- 垃圾回收
- 动态链接

## 内存管理基础

### 内存布局

程序运行时的内存通常分为以下几个区域：

```
+---------------------+  高地址
|      命令行参数      |
|       环境变量       |
+---------------------+
|        栈区         |  ↓
|   (局部变量、函数参数) |
|                     |
+---------------------+
|        堆区         |  ↑
|    (动态分配内存)    |
|                     |
+---------------------+
|       BSS段         |  未初始化的全局变量
+---------------------+
|       数据段        |  已初始化的全局变量
+---------------------+
|       代码段        |  程序指令
+---------------------+  低地址
```

### 栈内存管理

栈是一种后进先出（LIFO）的数据结构，用于管理函数调用和局部变量。

#### 栈帧结构

```c
typedef struct StackFrame {
    // 返回地址
    void* return_address;

    // 旧的帧指针
    struct StackFrame* previous_frame;

    // 局部变量
    char local_variables[];

    // 保存的寄存器
    Register saved_registers[];
} StackFrame;

// 栈帧布局示例（x86-64）
// +----------------------+  <-- rbp (帧指针)
// |     旧 rbp           |
// +----------------------+
// |     返回地址         |
// +----------------------+
// |   局部变量1          |
// +----------------------+
// |   局部变量2          |
// +----------------------+
// |   保存的寄存器       |
// +----------------------+  <-- rsp (栈指针)
```

#### 栈操作实现

```c
// 函数调用时的栈操作
void function_prologue() {
    asm volatile (
        "push rbp\n"      // 保存旧的帧指针
        "mov rbp, rsp\n"  // 设置新的帧指针
        "sub rsp, %0\n"   // 分配局部变量空间
        :
        : "r"(LOCAL_VARIABLE_SIZE)
    );
}

void function_epilogue() {
    asm volatile (
        "mov rsp, rbp\n"  // 释放局部变量空间
        "pop rbp\n"       // 恢复旧的帧指针
        "ret\n"           // 返回
    );
}

// 分配栈空间
void* allocate_stack_space(size_t size) {
    void* ptr;
    asm volatile (
        "sub %%rsp, %1\n"
        "mov %%rsp, %0"
        : "=r"(ptr)
        : "r"(size)
        : "memory"
    );
    return ptr;
}

// 释放栈空间
void free_stack_space(size_t size) {
    asm volatile (
        "add %%rsp, %0"
        :
        : "r"(size)
        : "memory"
    );
}
```

### 堆内存管理

堆用于动态内存分配，程序员可以请求任意大小的内存块。

#### 简单的堆分配器

```c
typedef struct BlockHeader {
    size_t size;              // 块大小
    bool is_free;             // 是否空闲
    struct BlockHeader* next; // 下一个块
} BlockHeader;

typedef struct Heap {
    BlockHeader* free_list;   // 空闲块链表
    void* start;              // 堆起始地址
    void* end;                // 堆结束地址
    size_t total_size;        // 堆总大小
    size_t allocated_size;    // 已分配大小
} Heap;

Heap* create_heap(size_t size) {
    Heap* heap = malloc(sizeof(Heap));
    heap->start = malloc(size);
    heap->end = (char*)heap->start + size;
    heap->total_size = size;
    heap->allocated_size = 0;

    // 初始化空闲块
    BlockHeader* initial_block = (BlockHeader*)heap->start;
    initial_block->size = size - sizeof(BlockHeader);
    initial_block->is_free = true;
    initial_block->next = NULL;
    heap->free_list = initial_block;

    return heap;
}

void* heap_malloc(Heap* heap, size_t size) {
    // 对齐大小
    size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);

    // 首次适应算法
    BlockHeader* current = heap->free_list;
    BlockHeader* prev = NULL;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // 找到合适的块
            if (current->size >= size + sizeof(BlockHeader) + 16) {
                // 分割块
                BlockHeader* new_block = (BlockHeader*)((char*)current +
                                                      sizeof(BlockHeader) + size);
                new_block->size = current->size - size - sizeof(BlockHeader);
                new_block->is_free = true;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            current->is_free = false;
            heap->allocated_size += size;

            return (char*)current + sizeof(BlockHeader);
        }

        prev = current;
        current = current->next;
    }

    // 没有足够的内存
    return NULL;
}

void heap_free(Heap* heap, void* ptr) {
    if (ptr == NULL) return;

    BlockHeader* block = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    block->is_free = true;
    heap->allocated_size -= block->size;

    // 合并相邻的空闲块
    coalesce_free_blocks(heap);
}

void coalesce_free_blocks(Heap* heap) {
    BlockHeader* current = heap->free_list;

    while (current != NULL && current->next != NULL) {
        if (current->is_free && current->next->is_free) {
            // 合并块
            current->size += current->next->size + sizeof(BlockHeader);
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}
```

## 垃圾回收

垃圾回收（Garbage Collection, GC）是自动内存管理的核心机制，它自动识别并回收不再使用的内存。

### 引用计数

引用计数是最简单的垃圾回收算法，每个对象维护一个引用计数器。

```c
typedef struct GCObject {
    int ref_count;              // 引用计数
    void (*destructor)(void*);  // 析构函数
    struct GCObject* next;      // 链表指针
    // 对象数据...
} GCObject;

GCObject* gc_object_list = NULL;

void* gc_malloc(size_t size) {
    GCObject* obj = malloc(sizeof(GCObject) + size);
    obj->ref_count = 1;
    obj->destructor = NULL;

    // 添加到对象列表
    obj->next = gc_object_list;
    gc_object_list = obj;

    return (char*)obj + sizeof(GCObject);
}

void gc_retain(void* ptr) {
    if (ptr == NULL) return;

    GCObject* obj = (GCObject*)((char*)ptr - sizeof(GCObject));
    obj->ref_count++;
}

void gc_release(void* ptr) {
    if (ptr == NULL) return;

    GCObject* obj = (GCObject*)((char*)ptr - sizeof(GCObject));
    obj->ref_count--;

    if (obj->ref_count == 0) {
        // 调用析构函数
        if (obj->destructor) {
            obj->destructor(ptr);
        }

        // 从对象列表中移除
        remove_from_object_list(obj);

        // 释放内存
        free(obj);
    }
}
```

引用计数的问题：
- **循环引用**：两个或多个对象相互引用，导致内存泄漏
- **性能开销**：每次引用修改都需要更新计数器

### 标记-清除（Mark and Sweep）

标记-清除算法分为两个阶段：标记阶段和清除阶段。

```c
typedef struct Object {
    bool marked;               // 标记位
    struct Object** children;  // 引用的子对象
    int child_count;           // 子对象数量
    struct Object* next;       // 链表指针
    // 对象数据...
} Object;

Object* root_objects = NULL;
Object* all_objects = NULL;

// 标记阶段
void mark_phase() {
    // 清除所有标记
    for (Object* obj = all_objects; obj != NULL; obj = obj->next) {
        obj->marked = false;
    }

    // 从根对象开始标记
    for (Object* root = root_objects; root != NULL; root = root->next) {
        mark_object(root);
    }
}

void mark_object(Object* obj) {
    if (obj == NULL || obj->marked) return;

    obj->marked = true;

    // 递归标记所有子对象
    for (int i = 0; i < obj->child_count; i++) {
        mark_object(obj->children[i]);
    }
}

// 清除阶段
void sweep_phase() {
    Object** prev = &all_objects;
    Object* current = all_objects;

    while (current != NULL) {
        if (!current->marked) {
            // 删除未标记的对象
            *prev = current->next;
            free_object(current);
            current = *prev;
        } else {
            prev = &current->next;
            current = current->next;
        }
    }
}

void collect_garbage() {
    mark_phase();
    sweep_phase();
}
```

### 复制算法

复制算法将堆分为两个半空间，每次只使用其中一个。

```c
typedef struct CopyingGC {
    void* from_space;          // 源空间
    void* to_space;            // 目标空间
    size_t space_size;         // 每个空间的大小
    void* allocation_pointer;  // 分配指针
} CopyingGC;

CopyingGC* create_copying_gc(size_t heap_size) {
    CopyingGC* gc = malloc(sizeof(CopyingGC));
    size_t space_size = heap_size / 2;

    gc->from_space = malloc(heap_size);
    gc->to_space = (char*)gc->from_space + space_size;
    gc->space_size = space_size;
    gc->allocation_pointer = gc->from_space;

    return gc;
}

void* gc_allocate(CopyingGC* gc, size_t size) {
    // 对齐大小
    size = (size + 7) & ~7;

    // 检查是否有足够空间
    if ((char*)gc->allocation_pointer + size >
        (char*)gc->from_space + gc->space_size) {
        // 触发垃圾回收
        gc_collect(gc);

        // 再次检查空间
        if ((char*)gc->allocation_pointer + size >
            (char*)gc->from_space + gc->space_size) {
            return NULL; // 内存不足
        }
    }

    // 分配内存
    void* ptr = gc->allocation_pointer;
    gc->allocation_pointer = (char*)gc->allocation_pointer + size;
    return ptr;
}

void gc_collect(CopyingGC* gc) {
    // 交换from和to空间
    void* temp = gc->from_space;
    gc->from_space = gc->to_space;
    gc->to_space = temp;

    // 重置分配指针
    gc->allocation_pointer = gc->from_space;

    // 复制存活对象
    for (Object* root = get_root_objects(); root != NULL; root = root->next) {
        copy_object(gc, root);
    }
}

void* copy_object(CopyingGC* gc, Object* obj) {
    // 检查对象是否已经在to空间中（转发指针）
    if (is_forwarded(obj)) {
        return get_forwarding_address(obj);
    }

    // 在to空间中分配新位置
    void* new_location = gc->allocation_pointer;
    size_t object_size = get_object_size(obj);
    gc->allocation_pointer = (char*)gc->allocation_pointer + object_size;

    // 复制对象
    memcpy(new_location, obj, object_size);

    // 设置转发指针
    set_forwarding_pointer(obj, new_location);

    // 递归复制子对象
    Object* new_obj = (Object*)new_location;
    for (int i = 0; i < new_obj->child_count; i++) {
        new_obj->children[i] = copy_object(gc, new_obj->children[i]);
    }

    return new_location;
}
```

### 分代垃圾回收

分代垃圾回收基于"分代假说"：大多数对象很快就不再使用，而存活时间长的对象很少会死掉。

```c
typedef struct GenerationalGC {
    // 年轻代
    struct {
        void* eden_space;
        void* survivor_space[2];
        int survivor_index;
        size_t young_generation_size;
    } young;

    // 老年代
    struct {
        void* old_space;
        size_t old_generation_size;
    } old;

    int promotion_threshold;    // 晋升阈值
} GenerationalGC;

void young_gc(GenerationalGC* gc) {
    // 只在年轻代进行垃圾回收
    copy_to_survivor_space(gc);

    // 处理幸存者空间
    if (should_promote_objects(gc)) {
        promote_to_old_generation(gc);
    } else {
        swap_survivor_spaces(gc);
    }
}

void full_gc(GenerationalGC* gc) {
    // 全堆垃圾回收
    mark_sweep_old_generation(gc);
    young_gc(gc);
}

bool should_promote_objects(GenerationalGC* gc) {
    // 根据对象年龄或其他条件决定是否晋升
    return get_survivor_age(gc) > gc->promotion_threshold;
}
```

## 异常处理

异常处理提供了结构化的错误处理机制，使程序能够优雅地处理运行时错误。

### 异常处理机制

```c
typedef struct ExceptionHandler {
    struct ExceptionHandler* next;     // 上一个处理器
    void* stack_pointer;               // 保存的栈指针
    void* frame_pointer;               // 保存的帧指针
    void* jump_buffer[5];              // 跳转缓冲区
    void (*handler)(int exception_code); // 异常处理函数
} ExceptionHandler;

ExceptionHandler* current_handler = NULL;

// 设置异常处理器
void set_exception_handler(ExceptionHandler* handler,
                         void (*exception_handler)(int)) {
    handler->next = current_handler;
    handler->handler = exception_handler;

    // 保存当前寄存器状态
    asm volatile (
        "mov %%rsp, %0\n"
        "mov %%rbp, %1"
        : "=m"(handler->stack_pointer),
          "=m"(handler->frame_pointer)
    );

    current_handler = handler;
}

// 抛出异常
void throw_exception(int exception_code) {
    if (current_handler == NULL) {
        // 没有异常处理器，终止程序
        fprintf(stderr, "Uncaught exception: %d\n", exception_code);
        exit(1);
    }

    // 恢复处理器状态
    asm volatile (
        "mov %0, %%rsp\n"
        "mov %1, %%rbp"
        :
        : "m"(current_handler->stack_pointer),
          "m"(current_handler->frame_pointer)
    );

    // 调用异常处理函数
    current_handler->handler(exception_code);
}

// 清理异常处理器
void cleanup_exception_handler(ExceptionHandler* handler) {
    current_handler = handler->next;
}

// 使用示例
void risky_function() {
    ExceptionHandler handler;
    set_exception_handler(&handler, handle_exception);

    // 可能抛出异常的代码
    if (error_condition) {
        throw_exception(42);
    }

    cleanup_exception_handler(&handler);
}

void handle_exception(int exception_code) {
    printf("Caught exception: %d\n", exception_code);
    // 处理异常后，执行会继续
}
```

### 异常对象和类型安全

```c
typedef struct Exception {
    const char* type;
    const char* message;
    void* data;
    struct Exception* cause;
} Exception;

void throw_exception_object(Exception* exception) {
    if (current_handler == NULL) {
        // 没有异常处理器
        print_exception(exception);
        exit(1);
    }

    // 恢复处理器状态并调用处理函数
    asm volatile (
        "mov %0, %%rsp\n"
        "mov %1, %%rbp"
        :
        : "m"(current_handler->stack_pointer),
          "m"(current_handler->frame_pointer)
    );

    // 调用类型安全的异常处理函数
    ((void(*)(Exception*))current_handler->handler)(exception);
}

// 类型安全的异常捕获
#define TRY(handler_var, exception_type) \
    ExceptionHandler handler_var; \
    set_exception_handler(&handler_var, (void*)handle_##exception_type); \
    int __caught = 0;

#define CATCH(handler_var, exception_var) \
    cleanup_exception_handler(&handler_var); \
    if (__caught) { \
        exception_type* exception_var = (exception_type*)__exception_object; \
        __caught = 0;

#define END_CATCH \
    }

void handle_file_exception(Exception* exception) {
    if (strcmp(exception->type, "FileException") == 0) {
        __exception_object = exception;
        __caught = 1;
        return;
    }
    // 不是我们要处理的异常类型，继续向上传播
    throw_exception_object(exception);
}
```

## 动态类型系统

动态类型系统在运行时进行类型检查和转换。

### 类型信息和运行时类型识别（RTTI）

```c
typedef struct TypeInfo {
    const char* name;
    size_t size;
    struct TypeInfo* base_type;
    bool (*is_compatible)(struct TypeInfo* other);
    void* (*cast)(void* obj, struct TypeInfo* target_type);
} TypeInfo;

typedef struct Object {
    TypeInfo* type;
    int ref_count;
    // 对象数据...
} Object;

// 类型检查函数
bool is_instance_of(Object* obj, TypeInfo* type) {
    if (obj == NULL) return false;

    TypeInfo* current_type = obj->type;
    while (current_type != NULL) {
        if (current_type == type) {
            return true;
        }
        current_type = current_type->base_type;
    }

    return false;
}

// 安全类型转换
void* safe_cast(Object* obj, TypeInfo* target_type) {
    if (obj == NULL) return NULL;

    if (is_instance_of(obj, target_type)) {
        return obj;
    }

    // 类型转换失败
    throw_type_error(obj->type, target_type);
    return NULL;
}

void throw_type_error(TypeInfo* source_type, TypeInfo* target_type) {
    Exception* exception = malloc(sizeof(Exception));
    exception->type = "TypeError";
    exception->message = malloc(256);
    sprintf((char*)exception->message,
            "Cannot cast from %s to %s",
            source_type->name, target_type->name);
    exception->data = NULL;
    exception->cause = NULL;

    throw_exception_object(exception);
}
```

### 动态方法调用

```c
typedef struct Method {
    const char* name;
    void* (*function)(Object* self, void* args[]);
    TypeInfo* param_types[10];
    int param_count;
} Method;

typedef struct VTable {
    Method* methods;
    int method_count;
} VTable;

// 动态方法调用
void* dynamic_method_call(Object* obj, const char* method_name, void* args[]) {
    VTable* vtable = get_vtable(obj->type);

    // 查找方法
    Method* method = find_method(vtable, method_name);
    if (method == NULL) {
        throw_method_not_found(obj->type, method_name);
        return NULL;
    }

    // 检查参数类型
    for (int i = 0; i < method->param_count; i++) {
        if (args[i] != NULL) {
            Object* arg_obj = (Object*)args[i];
            if (!is_instance_of(arg_obj, method->param_types[i])) {
                throw_argument_type_error(method, i,
                                        arg_obj->type,
                                        method->param_types[i]);
                return NULL;
            }
        }
    }

    // 调用方法
    return method->function(obj, args);
}

Method* find_method(VTable* vtable, const char* method_name) {
    for (int i = 0; i < vtable->method_count; i++) {
        if (strcmp(vtable->methods[i].name, method_name) == 0) {
            return &vtable->methods[i];
        }
    }

    // 在父类中查找
    if (vtable->base_vtable != NULL) {
        return find_method(vtable->base_vtable, method_name);
    }

    return NULL;
}
```

## 线程和并发支持

### 线程管理

```c
typedef struct Thread {
    int id;
    void* stack_pointer;
    void* (*function)(void*);
    void* argument;
    enum {
        THREAD_READY,
        THREAD_RUNNING,
        THREAD_BLOCKED,
        THREAD_TERMINATED
    } state;
    struct Thread* next;
} Thread;

typedef struct Scheduler {
    Thread* ready_queue;
    Thread* current_thread;
    int thread_count;
    void* context;
} Scheduler;

Scheduler* create_scheduler() {
    Scheduler* scheduler = malloc(sizeof(Scheduler));
    scheduler->ready_queue = NULL;
    scheduler->current_thread = NULL;
    scheduler->thread_count = 0;
    scheduler->context = NULL;
    return scheduler;
}

Thread* create_thread(void (*function)(void*), void* argument) {
    Thread* thread = malloc(sizeof(Thread));
    thread->id = generate_thread_id();
    thread->function = function;
    thread->argument = argument;
    thread->state = THREAD_READY;
    thread->stack_pointer = allocate_thread_stack();

    // 设置线程栈
    setup_thread_stack(thread);

    return thread;
}

void schedule_thread(Scheduler* scheduler) {
    if (scheduler->ready_queue == NULL) {
        // 没有就绪的线程
        return;
    }

    Thread* next_thread = scheduler->ready_queue;
    scheduler->ready_queue = next_thread->next;

    if (scheduler->current_thread != NULL) {
        // 保存当前线程状态
        save_thread_context(scheduler->current_thread);

        // 将当前线程重新加入就绪队列
        if (scheduler->current_thread->state == THREAD_RUNNING) {
            scheduler->current_thread->state = THREAD_READY;
            add_to_ready_queue(scheduler, scheduler->current_thread);
        }
    }

    // 切换到新线程
    scheduler->current_thread = next_thread;
    next_thread->state = THREAD_RUNNING;
    restore_thread_context(next_thread);
}

void thread_yield() {
    Scheduler* scheduler = get_current_scheduler();
    schedule_thread(scheduler);
}
```

### 同步原语

```c
// 互斥锁
typedef struct Mutex {
    volatile int locked;
    Thread* owner;
    Thread* wait_queue;
} Mutex;

Mutex* create_mutex() {
    Mutex* mutex = malloc(sizeof(Mutex));
    mutex->locked = 0;
    mutex->owner = NULL;
    mutex->wait_queue = NULL;
    return mutex;
}

void mutex_lock(Mutex* mutex) {
    Thread* current_thread = get_current_thread();

    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        // 锁被占用，等待
        if (mutex->owner == current_thread) {
            // 重入锁
            return;
        }

        // 添加到等待队列
        add_to_wait_queue(mutex, current_thread);
        thread_yield();
    }

    mutex->owner = current_thread;
}

void mutex_unlock(Mutex* mutex) {
    Thread* current_thread = get_current_thread();

    if (mutex->owner != current_thread) {
        // 不是锁的所有者
        return;
    }

    // 释放锁
    mutex->locked = 0;
    mutex->owner = NULL;

    // 唤醒等待的线程
    if (mutex->wait_queue != NULL) {
        Thread* next_thread = mutex->wait_queue;
        mutex->wait_queue = next_thread->next;
        add_to_ready_queue(get_current_scheduler(), next_thread);
    }
}

// 条件变量
typedef struct ConditionVariable {
    Thread* wait_queue;
    Mutex* associated_mutex;
} ConditionVariable;

void condition_wait(ConditionVariable* cond, Mutex* mutex) {
    Thread* current_thread = get_current_thread();

    // 释放互斥锁
    mutex_unlock(mutex);

    // 添加到等待队列
    add_to_wait_queue(cond, current_thread);
    thread_yield();

    // 被唤醒后重新获取锁
    mutex_lock(mutex);
}

void condition_signal(ConditionVariable* cond) {
    if (cond->wait_queue != NULL) {
        Thread* waiting_thread = cond->wait_queue;
        cond->wait_queue = waiting_thread->next;

        add_to_ready_queue(get_current_scheduler(), waiting_thread);
    }
}

void condition_broadcast(ConditionVariable* cond) {
    while (cond->wait_queue != NULL) {
        condition_signal(cond);
    }
}
```

## 性能监控和调试

### 内存监控

```c
typedef struct MemoryStats {
    size_t total_allocated;
    size_t total_freed;
    size_t current_usage;
    int allocation_count;
    int free_count;
    size_t peak_usage;
} MemoryStats;

MemoryStats memory_stats = {0};

void* monitored_malloc(size_t size) {
    void* ptr = malloc(size);

    if (ptr != NULL) {
        memory_stats.total_allocated += size;
        memory_stats.current_usage += size;
        memory_stats.allocation_count++;

        if (memory_stats.current_usage > memory_stats.peak_usage) {
            memory_stats.peak_usage = memory_stats.current_usage;
        }
    }

    return ptr;
}

void monitored_free(void* ptr, size_t size) {
    free(ptr);

    memory_stats.total_freed += size;
    memory_stats.current_usage -= size;
    memory_stats.free_count++;
}

void print_memory_stats() {
    printf("Memory Statistics:\n");
    printf("  Total allocated: %zu bytes\n", memory_stats.total_allocated);
    printf("  Total freed: %zu bytes\n", memory_stats.total_freed);
    printf("  Current usage: %zu bytes\n", memory_stats.current_usage);
    printf("  Peak usage: %zu bytes\n", memory_stats.peak_usage);
    printf("  Allocation count: %d\n", memory_stats.allocation_count);
    printf("  Free count: %d\n", memory_stats.free_count);
}

// 内存泄漏检测
typedef struct AllocationRecord {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    struct AllocationRecord* next;
} AllocationRecord;

AllocationRecord* allocation_records = NULL;

void* debug_malloc(size_t size, const char* file, int line) {
    void* ptr = monitored_malloc(size);

    if (ptr != NULL) {
        AllocationRecord* record = malloc(sizeof(AllocationRecord));
        record->ptr = ptr;
        record->size = size;
        record->file = file;
        record->line = line;
        record->next = allocation_records;
        allocation_records = record;
    }

    return ptr;
}

void debug_free(void* ptr) {
    // 查找并删除分配记录
    AllocationRecord** current = &allocation_records;
    while (*current != NULL) {
        if ((*current)->ptr == ptr) {
            AllocationRecord* to_delete = *current;
            *current = (*current)->next;

            monitored_free(ptr, to_delete->size);
            free(to_delete);
            return;
        }
        current = &(*current)->next;
    }

    // 没有找到分配记录，可能是重复释放
    fprintf(stderr, "Warning: Freeing unallocated pointer %p\n", ptr);
}

void check_memory_leaks() {
    if (allocation_records != NULL) {
        printf("Memory leaks detected:\n");

        AllocationRecord* current = allocation_records;
        while (current != NULL) {
            printf("  Leak: %zu bytes allocated at %s:%d\n",
                   current->size, current->file, current->line);
            current = current->next;
        }
    } else {
        printf("No memory leaks detected.\n");
    }
}

#define MALLOC(size) debug_malloc(size, __FILE__, __LINE__)
#define FREE(ptr) debug_free(ptr)
```

### 性能分析

```c
typedef struct ProfileEntry {
    const char* function_name;
    uint64_t call_count;
    uint64_t total_time;
    uint64_t min_time;
    uint64_t max_time;
} ProfileEntry;

#define MAX_PROFILE_ENTRIES 1000
ProfileEntry profile_entries[MAX_PROFILE_ENTRIES];
int profile_entry_count = 0;

uint64_t get_timestamp() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

ProfileEntry* get_profile_entry(const char* function_name) {
    for (int i = 0; i < profile_entry_count; i++) {
        if (strcmp(profile_entries[i].function_name, function_name) == 0) {
            return &profile_entries[i];
        }
    }

    if (profile_entry_count < MAX_PROFILE_ENTRIES) {
        ProfileEntry* entry = &profile_entries[profile_entry_count++];
        entry->function_name = function_name;
        entry->call_count = 0;
        entry->total_time = 0;
        entry->min_time = UINT64_MAX;
        entry->max_time = 0;
        return entry;
    }

    return NULL;
}

#define PROFILE_FUNCTION(name) \
    static const char* __profile_name = name; \
    uint64_t __start_time = get_timestamp();

#define PROFILE_END() \
    do { \
        uint64_t __end_time = get_timestamp(); \
        uint64_t __duration = __end_time - __start_time; \
        ProfileEntry* __entry = get_profile_entry(__profile_name); \
        if (__entry != NULL) { \
            __entry->call_count++; \
            __entry->total_time += __duration; \
            if (__duration < __entry->min_time) __entry->min_time = __duration; \
            if (__duration > __entry->max_time) __entry->max_time = __duration; \
        } \
    } while(0)

void print_profile_stats() {
    printf("Profile Statistics:\n");
    printf("%-20s %10s %15s %15s %15s\n",
           "Function", "Calls", "Total Time", "Avg Time", "Max Time");
    printf("%-20s %10s %15s %15s %15s\n",
           "--------------------", "----------", "---------------",
           "---------------", "---------------");

    for (int i = 0; i < profile_entry_count; i++) {
        ProfileEntry* entry = &profile_entries[i];
        uint64_t avg_time = entry->total_time / entry->call_count;

        printf("%-20s %10lu %15lu %15lu %15lu\n",
               entry->function_name,
               entry->call_count,
               entry->total_time,
               avg_time,
               entry->max_time);
    }
}
```

## 实际案例分析

### 简单语言的运行时系统

让我们为一个简单的编程语言设计运行时系统：

**语言特性**：
- 基本数据类型：int, float, bool, string
- 复合数据类型：数组、结构体
- 动态内存管理
- 异常处理
- 垃圾回收

```c
// 运行时系统初始化
typedef struct Runtime {
    Heap* heap;
    GarbageCollector* gc;
    ExceptionHandler* exception_stack;
    TypeRegistry* type_registry;
    MemoryStats* memory_stats;
} Runtime;

Runtime* runtime = NULL;

void runtime_init() {
    runtime = malloc(sizeof(Runtime));

    // 初始化堆（1MB）
    runtime->heap = create_heap(1024 * 1024);

    // 初始化垃圾收集器
    runtime->gc = create_garbage_collector(COPYING_GC);

    // 初始化异常处理
    runtime->exception_stack = NULL;

    // 初始化类型系统
    runtime->type_registry = create_type_registry();

    // 初始化内存统计
    runtime->memory_stats = create_memory_stats();

    // 注册内置类型
    register_builtin_types(runtime->type_registry);
}

// 对象创建
Object* create_object(TypeInfo* type) {
    size_t size = sizeof(Object) + type->size;
    Object* obj = gc_allocate(runtime->gc, size);

    obj->type = type;
    obj->ref_count = 1;

    // 调用构造函数
    if (type->constructor != NULL) {
        type->constructor(obj);
    }

    return obj;
}

// 异常处理API
void runtime_throw(const char* exception_type, const char* message) {
    Exception* exception = create_exception(exception_type, message);
    runtime_throw_exception(exception);
}

// 示例：数组越界检查
int array_get(Object* array_obj, int index) {
    TypeInfo* array_type = array_obj->type;

    if (strcmp(array_type->name, "Array") != 0) {
        runtime_throw("TypeError", "Object is not an array");
        return 0;
    }

    ArrayData* array_data = (ArrayData*)((char*)array_obj + sizeof(Object));

    if (index < 0 || index >= array_data->length) {
        runtime_throw("IndexError", "Array index out of bounds");
        return 0;
    }

    return array_data->elements[index];
}

// 垃圾回收触发
void trigger_gc_if_needed() {
    if (should_trigger_gc(runtime->gc)) {
        gc_collect(runtime->gc);
        update_memory_stats(runtime->memory_stats);
    }
}
```

### 性能优化示例

**优化前**：
```c
// 大量小对象分配
for (int i = 0; i < 100000; i++) {
    Object* obj = create_object(get_type("Point"));
    set_point_data(obj, i, i * 2);
    process_point(obj);
    // 对象在循环结束后成为垃圾
}
```

**优化后**：
```c
// 对象池化
ObjectPool* point_pool = create_object_pool(get_type("Point"), 1000);

for (int i = 0; i < 100000; i++) {
    Object* obj = pool_acquire(point_pool);
    set_point_data(obj, i, i * 2);
    process_point(obj);
    pool_release(point_pool, obj);  // 回收到池中而不是GC
}
```

## 总结

运行时系统和内存管理是现代编程语言的核心组成部分，它们为程序提供了执行环境和自动化服务。一个设计良好的运行时系统应该具备：

1. **高效的内存管理**：快速的内存分配和智能的垃圾回收
2. **强大的异常处理**：结构化的错误处理和恢复机制
3. **灵活的类型系统**：支持动态类型检查和安全的类型转换
4. **并发支持**：线程管理和同步原语
5. **调试和监控**：内存泄漏检测、性能分析等工具

### 设计原则

- **性能**：最小化运行时开销
- **可靠性**：正确处理各种边界情况
- **可扩展性**：支持不同的编程语言特性
- **可移植性**：适应不同的硬件平台和操作系统
- **可调试性**：提供丰富的调试和分析工具

### 现代发展趋势

- **即时编译（JIT）**：运行时编译热点代码
- **无锁数据结构**：提高并发性能
- **区域类型系统**：编译时内存管理
- **协程和异步I/O**：高效的并发编程
- **WebAssembly支持**：跨平台运行时

理解运行时系统的工作原理有助于我们：
- 设计更高效的编程语言
- 优化程序性能
- 调试复杂的内存问题
- 构建可靠的系统软件

通过这8篇技术博客，我们已经全面了解了编译器的各个组件和工作原理。从词法分析到目标代码生成，从优化技术到运行时系统，编译器是一个复杂而精妙的系统，它将人类的编程思想转换为机器可以执行的指令，是现代计算机科学的基石之一。