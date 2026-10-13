#include "wse_core.h"
#include "wse_operations.h"
#include <stdarg.h>
#include <chrono>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <mach/vm_map.h>
#include <sys/mman.h>
#include <libkern/OSCacheControl.h>

#define DYLD_INTERPOSE(_replacement, _replacee) \
   __attribute__((used)) static struct { const void* replacement; const void* replacee; } _interpose_##_replacee \
            __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacement, (const void*)(unsigned long)&_replacee };

WSE_VM g_vm;
WSE_ArrayManager g_arrays;
WSE_DictManager g_dicts;
WSE_TimerManager g_timers;
int32_t g_mouse_wheel_accumulated = 0;

// Target addresses in Mount & Blade x86_64 Mach-O
static const uintptr_t STATIC_UNRECOGNIZED_OPCODE_HANDLER = 0x100ad5747;
static const uintptr_t STATIC_STATEMENT_DONE              = 0x100adcd5d;
static const uintptr_t STATIC_ERROR_STRING                = 0x100ba2083;

// Trampoline symbols from wse_trampoline.s
extern "C" {
    void wse_opcode_trampoline();
    extern uintptr_t g_statement_done_target;
    extern uintptr_t g_unhandled_target;
    extern uintptr_t g_error_string_ptr;
    int wse_handle_opcode_dispatch(void* rbp);
    int SDL_PollEvent(void* event);
}

// Interpose SDL_PollEvent to capture mouse wheel events in real-time
static int hooked_SDL_PollEvent(void* event) {
    int res = SDL_PollEvent(event);
    if (res && event) {
        uint32_t type = *(uint32_t*)event;
        // SDL_MOUSEWHEEL is event type 0x403 (1027 in SDL2)
        if (type == 0x403) {
            // Offset to int32_t y (vertical wheel delta) in SDL_MouseWheelEvent is +20 bytes
            int32_t dy = *(int32_t*)((char*)event + 20);
            g_mouse_wheel_accumulated += dy;
        }
    }
    return res;
}
DYLD_INTERPOSE(hooked_SDL_PollEvent, SDL_PollEvent)

void WSE_VM::init(uintptr_t aslr_slide) {
    slide = aslr_slide;
    g_globals_ptr   = (int64_t**)(0x10145ce78 + slide);
    g_registers_ptr = (int64_t**)(0x10145d720 + slide);
    log("[WSE-VM] Initialized with ASLR slide 0x%lx\n", slide);
}

void WSE_VM::log(const char* fmt, ...) {
    FILE* f = fopen("/tmp/wse_mac.log", "a");
    if (!f) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fclose(f);
}

int64_t WSE_VM::eval_operand(int64_t raw_op, int64_t* local_vars) {
    if (raw_op < 0) return raw_op;

    uint8_t tag = (raw_op >> 56) & 0x7F;
    int64_t idx = raw_op & 0x00FFFFFFFFFFFFFF;

    switch (tag) {
        case 0:  // Immediate integer constant
            return idx;

        case 1:  // Global variable ($var)
            if (g_globals_ptr && *g_globals_ptr) {
                return (*g_globals_ptr)[idx + 0x118];
            }
            return 0;

        case 2:  // System registers (reg0..reg63)
            if (g_registers_ptr && *g_registers_ptr) {
                return (*g_registers_ptr)[idx];
            }
            return 0;

        case 17: // Local variable (:var, 0x11)
            if (local_vars) {
                return local_vars[idx];
            }
            return 0;

        default:
            return idx;
    }
}

void WSE_VM::assign_operand(int64_t raw_op, int64_t val, int64_t* local_vars) {
    uint8_t tag = (raw_op >> 56) & 0x7F;
    int64_t idx = raw_op & 0x00FFFFFFFFFFFFFF;

    if (tag == 17 && local_vars) {
        local_vars[idx] = val;
    } else if (tag == 2 && g_registers_ptr && *g_registers_ptr) {
        (*g_registers_ptr)[idx] = val;
    } else if (tag == 1 && g_globals_ptr && *g_globals_ptr) {
        (*g_globals_ptr)[idx + 0x118] = val;
    }
}

// Array Manager Implementation
int32_t WSE_ArrayManager::create(size_t initial_size) {
    int32_t id = next_id_++;
    arrays_[id].resize(initial_size, 0);
    return id;
}

void WSE_ArrayManager::set(int32_t arr_id, size_t index, int64_t val) {
    auto it = arrays_.find(arr_id);
    if (it != arrays_.end()) {
        if (index >= it->second.size()) {
            it->second.resize(index + 1, 0);
        }
        it->second[index] = val;
    }
}

int64_t WSE_ArrayManager::get(int32_t arr_id, size_t index) {
    auto it = arrays_.find(arr_id);
    if (it != arrays_.end() && index < it->second.size()) {
        return it->second[index];
    }
    return 0;
}

int64_t WSE_ArrayManager::size(int32_t arr_id) {
    auto it = arrays_.find(arr_id);
    return (it != arrays_.end()) ? (int64_t)it->second.size() : 0;
}

void WSE_ArrayManager::clear(int32_t arr_id) {
    auto it = arrays_.find(arr_id);
    if (it != arrays_.end()) {
        it->second.clear();
    }
}

void WSE_ArrayManager::free_array(int32_t arr_id) {
    arrays_.erase(arr_id);
}

void WSE_ArrayManager::push_back(int32_t arr_id, int64_t val) {
    auto it = arrays_.find(arr_id);
    if (it != arrays_.end()) {
        it->second.push_back(val);
    }
}

// Dictionary Manager Implementation
int32_t WSE_DictManager::create() {
    int32_t id = next_id_++;
    dicts_[id] = std::unordered_map<int64_t, int64_t>();
    return id;
}

void WSE_DictManager::set(int32_t dict_id, int64_t key, int64_t val) {
    auto it = dicts_.find(dict_id);
    if (it != dicts_.end()) {
        it->second[key] = val;
    }
}

int64_t WSE_DictManager::get(int32_t dict_id, int64_t key) {
    auto it = dicts_.find(dict_id);
    if (it != dicts_.end()) {
        auto val_it = it->second.find(key);
        if (val_it != it->second.end()) {
            return val_it->second;
        }
    }
    return 0;
}

bool WSE_DictManager::has(int32_t dict_id, int64_t key) {
    auto it = dicts_.find(dict_id);
    if (it != dicts_.end()) {
        return it->second.find(key) != it->second.end();
    }
    return false;
}

void WSE_DictManager::clear(int32_t dict_id) {
    auto it = dicts_.find(dict_id);
    if (it != dicts_.end()) {
        it->second.clear();
    }
}

void WSE_DictManager::free_dict(int32_t dict_id) {
    dicts_.erase(dict_id);
}

// High-Precision Timer Manager
uint64_t WSE_TimerManager::get_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

int64_t WSE_TimerManager::get_delta_ms() {
    uint64_t now = get_time_ms();
    if (last_time_ms_ == 0) {
        last_time_ms_ = now;
        return 0;
    }
    int64_t delta = (int64_t)(now - last_time_ms_);
    last_time_ms_ = now;
    return delta;
}

// C-Bridge Dispatcher called from assembly trampoline
extern "C" int wse_handle_opcode_dispatch(void* rbp) {
    if (!rbp) return 0;

    void** stmt_ptr_location = (void**)((char*)rbp - 0x10a10);
    if (!stmt_ptr_location || !*stmt_ptr_location) return 0;

    int64_t* local_vars = *(int64_t**)((char*)rbp - 0x10a00);
    Statement* stmt = (Statement*)(*stmt_ptr_location);

    int32_t raw_op = stmt->opcode;
    int32_t clean_op = raw_op & 0x0FFFFFFF;

    return wse_execute_operation(clean_op, stmt, local_vars);
}

// Memory Patcher
static bool install_wse_hook(uintptr_t slide) {
    uintptr_t target_addr = STATIC_UNRECOGNIZED_OPCODE_HANDLER + slide;
    g_statement_done_target = STATIC_STATEMENT_DONE + slide;
    g_unhandled_target      = target_addr + 16;
    g_error_string_ptr      = STATIC_ERROR_STRING + slide;

    uintptr_t page_addr = target_addr & ~0xFFFULL;
    size_t page_size = 0x2000;

    kern_return_t kr = vm_protect(
        mach_task_self(),
        (vm_address_t)page_addr,
        page_size,
        FALSE,
        VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY
    );

    if (kr != KERN_SUCCESS) {
        if (mprotect((void*)page_addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
            g_vm.log("[WSE-Hook] ERROR: Memory protect failed!\n");
            return false;
        }
    }

    // Write 14-byte 64-bit indirect jump: FF 25 00 00 00 00 [target_addr_64]
    unsigned char patch[16];
    patch[0] = 0xFF; patch[1] = 0x25;
    patch[2] = 0x00; patch[3] = 0x00; patch[4] = 0x00; patch[5] = 0x00;
    uintptr_t trampoline_addr = (uintptr_t)&wse_opcode_trampoline;
    memcpy(&patch[6], &trampoline_addr, sizeof(uintptr_t));
    patch[14] = 0x90; patch[15] = 0x90;

    memcpy((void*)target_addr, patch, sizeof(patch));

    vm_protect(mach_task_self(), (vm_address_t)page_addr, page_size, FALSE, VM_PROT_READ | VM_PROT_EXECUTE);
    sys_dcache_flush((void*)target_addr, sizeof(patch));
    sys_icache_invalidate((void*)target_addr, sizeof(patch));

    g_vm.log("[WSE-Hook] >>> Warband Script Enhancer Mac initialized successfully! <<<\n");
    return true;
}

__attribute__((constructor))
static void wse_init_entry() {
    FILE* f = fopen("/tmp/wse_mac.log", "w");
    if (f) {
        fprintf(f, "========================================================\n");
        fprintf(f, "       WARBAND SCRIPT ENHANCER (WSE) MAC v1.0.0         \n");
        fprintf(f, "========================================================\n");
        fclose(f);
    }
    intptr_t slide = _dyld_get_image_vmaddr_slide(0);
    g_vm.init(slide);
    install_wse_hook(slide);
}
