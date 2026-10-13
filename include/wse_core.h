#ifndef WSE_CORE_H
#define WSE_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <string>

// Structure representing an active bytecode statement in 64-bit Warband
#pragma pack(push, 1)
struct Statement {
    int32_t opcode;            // +0x00: Operation code (with flags 0x40000000, 0x80000000)
    int32_t padding;           // +0x04: 4 bytes padding
    int64_t num_operands;      // +0x08: Number of operands
    int64_t operands[16];      // +0x10: Operands array
};
#pragma pack(pop)

// WSE Virtual Machine abstraction for reading/writing operands & registers
class WSE_VM {
public:
    uintptr_t slide = 0;
    int64_t** g_registers_ptr = nullptr;
    int64_t** g_globals_ptr   = nullptr;
    int64_t trigger_params[16] = {0};

    void init(uintptr_t aslr_slide);
    int64_t eval_operand(int64_t raw_op, int64_t* local_vars);
    void assign_operand(int64_t raw_op, int64_t val, int64_t* local_vars);
    void log(const char* fmt, ...);
};

extern WSE_VM g_vm;

// Dynamic Array Manager (Voxels, Inventories, Entity IDs)
class WSE_ArrayManager {
private:
    std::unordered_map<int32_t, std::vector<int64_t>> arrays_;
    int32_t next_id_ = 1;

public:
    int32_t create(size_t initial_size);
    void set(int32_t arr_id, size_t index, int64_t val);
    int64_t get(int32_t arr_id, size_t index);
    int64_t size(int32_t arr_id);
    void clear(int32_t arr_id);
    void free_array(int32_t arr_id);
    void push_back(int32_t arr_id, int64_t val);
};

extern WSE_ArrayManager g_arrays;

// Key-Value Dictionary Manager (Recipe registries, block traits, stats)
class WSE_DictManager {
private:
    std::unordered_map<int32_t, std::unordered_map<int64_t, int64_t>> dicts_;
    int32_t next_id_ = 1;

public:
    int32_t create();
    void set(int32_t dict_id, int64_t key, int64_t val);
    int64_t get(int32_t dict_id, int64_t key);
    bool has(int32_t dict_id, int64_t key);
    void clear(int32_t dict_id);
    void free_dict(int32_t dict_id);
};

extern WSE_DictManager g_dicts;

// High-Precision Timer Manager
class WSE_TimerManager {
private:
    uint64_t last_time_ms_ = 0;

public:
    uint64_t get_time_ms();
    int64_t get_delta_ms();
};

extern WSE_TimerManager g_timers;

// Input & Mouse Wheel State
extern int32_t g_mouse_wheel_accumulated;

// Dispatcher declarations
int wse_execute_operation(int clean_op, Statement* stmt, int64_t* local_vars);

#endif // WSE_CORE_H
