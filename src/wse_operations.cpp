#include "wse_core.h"
#include "wse_operations.h"
#include <cmath>
#include <algorithm>
#include <unordered_map>

// External SDL2 functions from Warband binary
extern "C" {
    const uint8_t* SDL_GetKeyboardState(int* numkeys);
    uint32_t SDL_GetMouseState(int* x, int* y);
}

// File handle store for modders
static std::unordered_map<int32_t, FILE*> g_open_files;
static int32_t g_next_file_id = 1;

int wse_execute_operation(int clean_op, Statement* stmt, int64_t* local_vars) {
    switch (clean_op) {
        // =====================================================================
        // TRIGGER PARAMETERS (2075 - 2080)
        // =====================================================================
        case WSE_OP_STORE_TRIGGER_PARAM_1:
        case WSE_OP_STORE_TRIGGER_PARAM_2:
        case WSE_OP_STORE_TRIGGER_PARAM_3:
        case WSE_OP_STORE_TRIGGER_PARAM_4:
        case WSE_OP_STORE_TRIGGER_PARAM_5:
        case WSE_OP_STORE_TRIGGER_PARAM_6: {
            int param_idx = clean_op - 2074; // 2075 -> 1, 2076 -> 2, etc.
            int64_t dest = stmt->operands[0];
            int64_t val = (param_idx < 16) ? g_vm.trigger_params[param_idx] : 0;
            g_vm.assign_operand(dest, val, local_vars);
            return 1;
        }

        // =====================================================================
        // DYNAMIC ARRAYS (Voxel Chunks, Crafting Slots, Inventories)
        // =====================================================================
        case WSE_OP_ARRAY_CREATE: {
            int64_t dest = stmt->operands[0];
            int64_t size = g_vm.eval_operand(stmt->operands[1], local_vars);
            int32_t arr_id = g_arrays.create((size_t)(size > 0 ? size : 0));
            g_vm.assign_operand(dest, arr_id, local_vars);
            return 1;
        }

        case WSE_OP_ARRAY_SET_VAL: {
            int32_t arr_id = (int32_t)g_vm.eval_operand(stmt->operands[0], local_vars);
            int64_t index  = g_vm.eval_operand(stmt->operands[1], local_vars);
            int64_t val    = g_vm.eval_operand(stmt->operands[2], local_vars);
            g_arrays.set(arr_id, (size_t)index, val);
            return 1;
        }

        case WSE_OP_ARRAY_GET_VAL: {
            int64_t dest   = stmt->operands[0];
            int32_t arr_id = (int32_t)g_vm.eval_operand(stmt->operands[1], local_vars);
            int64_t index  = g_vm.eval_operand(stmt->operands[2], local_vars);
            int64_t val    = g_arrays.get(arr_id, (size_t)index);
            g_vm.assign_operand(dest, val, local_vars);
            return 1;
        }

        case WSE_OP_ARRAY_SIZE: {
            int64_t dest   = stmt->operands[0];
            int32_t arr_id = (int32_t)g_vm.eval_operand(stmt->operands[1], local_vars);
            int64_t sz     = g_arrays.size(arr_id);
            g_vm.assign_operand(dest, sz, local_vars);
            return 1;
        }

        case WSE_OP_ARRAY_CLEAR: {
            int32_t arr_id = (int32_t)g_vm.eval_operand(stmt->operands[0], local_vars);
            g_arrays.clear(arr_id);
            return 1;
        }

        case WSE_OP_ARRAY_FREE: {
            int32_t arr_id = (int32_t)g_vm.eval_operand(stmt->operands[0], local_vars);
            g_arrays.free_array(arr_id);
            return 1;
        }

        case WSE_OP_ARRAY_PUSH_BACK: {
            int32_t arr_id = (int32_t)g_vm.eval_operand(stmt->operands[0], local_vars);
            int64_t val    = g_vm.eval_operand(stmt->operands[1], local_vars);
            g_arrays.push_back(arr_id, val);
            return 1;
        }

        // =====================================================================
        // KEY-VALUE DICTIONARIES (Recipe Registries, Item Metadata)
        // =====================================================================
        case WSE_OP_DICT_CREATE: {
            int64_t dest = stmt->operands[0];
            int32_t dict_id = g_dicts.create();
            g_vm.assign_operand(dest, dict_id, local_vars);
            return 1;
        }

        case WSE_OP_DICT_SET_VAL: {
            int32_t dict_id = (int32_t)g_vm.eval_operand(stmt->operands[0], local_vars);
            int64_t key     = g_vm.eval_operand(stmt->operands[1], local_vars);
            int64_t val     = g_vm.eval_operand(stmt->operands[2], local_vars);
            g_dicts.set(dict_id, key, val);
            return 1;
        }

        case WSE_OP_DICT_GET_VAL: {
            int64_t dest    = stmt->operands[0];
            int32_t dict_id = (int32_t)g_vm.eval_operand(stmt->operands[1], local_vars);
            int64_t key     = g_vm.eval_operand(stmt->operands[2], local_vars);
            int64_t val     = g_dicts.get(dict_id, key);
            g_vm.assign_operand(dest, val, local_vars);
            return 1;
        }

        case WSE_OP_DICT_HAS_KEY: {
            int64_t dest    = stmt->operands[0];
            int32_t dict_id = (int32_t)g_vm.eval_operand(stmt->operands[1], local_vars);
            int64_t key     = g_vm.eval_operand(stmt->operands[2], local_vars);
            int64_t exists  = g_dicts.has(dict_id, key) ? 1 : 0;
            g_vm.assign_operand(dest, exists, local_vars);
            return 1;
        }

        case WSE_OP_DICT_CLEAR: {
            int32_t dict_id = (int32_t)g_vm.eval_operand(stmt->operands[0], local_vars);
            g_dicts.clear(dict_id);
            return 1;
        }

        case WSE_OP_DICT_FREE: {
            int32_t dict_id = (int32_t)g_vm.eval_operand(stmt->operands[0], local_vars);
            g_dicts.free_dict(dict_id);
            return 1;
        }

        // =====================================================================
        // INPUT HOOKS (Direct macOS SDL2 Hardware Queries)
        // =====================================================================
        case WSE_OP_KEY_IS_DOWN: {
            int64_t dest = stmt->operands[0];
            int64_t key  = g_vm.eval_operand(stmt->operands[1], local_vars);
            int num_keys = 0;
            const uint8_t* state = SDL_GetKeyboardState(&num_keys);
            int is_down = (state && key >= 0 && key < num_keys) ? state[key] : 0;
            g_vm.assign_operand(dest, is_down, local_vars);
            return 1;
        }

        case WSE_OP_MOUSE_GET_POSITION: {
            int64_t dest_x = stmt->operands[0];
            int64_t dest_y = stmt->operands[1];
            int mx = 0, my = 0;
            SDL_GetMouseState(&mx, &my);
            g_vm.assign_operand(dest_x, mx, local_vars);
            g_vm.assign_operand(dest_y, my, local_vars);
            return 1;
        }

        case WSE_OP_MOUSE_GET_WHEEL_DELTA: {
            // mouse_get_wheel_delta <dest_delta>
            // Returns vertical scroll (+1 / -1) and resets accumulator for hotbars
            int64_t dest = stmt->operands[0];
            int32_t delta = g_mouse_wheel_accumulated;
            g_mouse_wheel_accumulated = 0; // Consume delta
            g_vm.assign_operand(dest, delta, local_vars);
            return 1;
        }

        case WSE_OP_MOUSE_IS_DOWN: {
            // mouse_is_down <dest>, <button: 1=left, 2=middle, 3=right>
            int64_t dest = stmt->operands[0];
            int64_t btn  = g_vm.eval_operand(stmt->operands[1], local_vars);
            int mx = 0, my = 0;
            uint32_t state = SDL_GetMouseState(&mx, &my);
            int is_down = 0;
            if (btn == 1) is_down = (state & (1 << 0)) ? 1 : 0; // SDL_BUTTON_LMASK
            else if (btn == 2) is_down = (state & (1 << 1)) ? 1 : 0; // SDL_BUTTON_MMASK
            else if (btn == 3) is_down = (state & (1 << 2)) ? 1 : 0; // SDL_BUTTON_RMASK
            g_vm.assign_operand(dest, is_down, local_vars);
            return 1;
        }

        // =====================================================================
        // HIGH-PRECISION TIMING (Mining Progress, Fluid Animations)
        // =====================================================================
        case WSE_OP_TIMER_GET_TIME_MS: {
            int64_t dest = stmt->operands[0];
            uint64_t ms = g_timers.get_time_ms();
            g_vm.assign_operand(dest, (int64_t)ms, local_vars);
            return 1;
        }

        case WSE_OP_TIMER_GET_DELTA_MS: {
            int64_t dest = stmt->operands[0];
            int64_t dt = g_timers.get_delta_ms();
            g_vm.assign_operand(dest, dt, local_vars);
            return 1;
        }

        // =====================================================================
        // 3D VECTOR MATH & VOXEL GRID SNAPPING
        // =====================================================================
        case WSE_OP_POSITION_SNAP_TO_GRID: {
            // position_snap_to_grid <dest_val>, <coord_val>, <grid_size_cm>
            // Snaps 3D world coordinates to voxel boundaries (e.g., 100cm cube)
            int64_t dest      = stmt->operands[0];
            int64_t coord     = g_vm.eval_operand(stmt->operands[1], local_vars);
            int64_t grid_size = g_vm.eval_operand(stmt->operands[2], local_vars);
            if (grid_size <= 0) grid_size = 100;
            int64_t snapped = (coord >= 0)
                ? ((coord + (grid_size / 2)) / grid_size) * grid_size
                : ((coord - (grid_size / 2)) / grid_size) * grid_size;
            g_vm.assign_operand(dest, snapped, local_vars);
            return 1;
        }

        case WSE_OP_POSITION_GET_DISTANCE_METERS: {
            // position_get_distance_meters <dest>, <x1>, <y1>, <z1>, <x2>, <y2>, <z2>
            int64_t dest = stmt->operands[0];
            double dx = (double)(g_vm.eval_operand(stmt->operands[1], local_vars) - g_vm.eval_operand(stmt->operands[4], local_vars));
            double dy = (double)(g_vm.eval_operand(stmt->operands[2], local_vars) - g_vm.eval_operand(stmt->operands[5], local_vars));
            double dz = (double)(g_vm.eval_operand(stmt->operands[3], local_vars) - g_vm.eval_operand(stmt->operands[6], local_vars));
            double dist_meters = std::sqrt(dx*dx + dy*dy + dz*dz) / 100.0;
            g_vm.assign_operand(dest, (int64_t)dist_meters, local_vars);
            return 1;
        }

        // =====================================================================
        // FILE I/O OPERATIONS
        // =====================================================================
        case WSE_OP_FILE_OPEN: {
            int64_t dest = stmt->operands[0];
            int64_t mode = g_vm.eval_operand(stmt->operands[1], local_vars);
            const char* mode_str = (mode == 1) ? "w" : ((mode == 2) ? "a" : "r");
            char path[256];
            snprintf(path, sizeof(path), "/tmp/wse_mod_data_%d.dat", g_next_file_id);
            FILE* f = fopen(path, mode_str);
            if (f) {
                int32_t fid = g_next_file_id++;
                g_open_files[fid] = f;
                g_vm.assign_operand(dest, fid, local_vars);
            } else {
                g_vm.assign_operand(dest, -1, local_vars);
            }
            return 1;
        }

        case WSE_OP_FILE_WRITE_INT: {
            int32_t fid = (int32_t)g_vm.eval_operand(stmt->operands[0], local_vars);
            int64_t val = g_vm.eval_operand(stmt->operands[1], local_vars);
            auto it = g_open_files.find(fid);
            if (it != g_open_files.end() && it->second) {
                fprintf(it->second, "%lld\n", val);
                fflush(it->second);
            }
            return 1;
        }

        case WSE_OP_FILE_READ_INT: {
            int64_t dest = stmt->operands[0];
            int32_t fid  = (int32_t)g_vm.eval_operand(stmt->operands[1], local_vars);
            int64_t val  = 0;
            auto it = g_open_files.find(fid);
            if (it != g_open_files.end() && it->second) {
                if (fscanf(it->second, "%lld", &val) != 1) val = 0;
            }
            g_vm.assign_operand(dest, val, local_vars);
            return 1;
        }

        case WSE_OP_FILE_CLOSE: {
            int32_t fid = (int32_t)g_vm.eval_operand(stmt->operands[0], local_vars);
            auto it = g_open_files.find(fid);
            if (it != g_open_files.end() && it->second) {
                fclose(it->second);
                g_open_files.erase(it);
            }
            return 1;
        }

        // =====================================================================
        // DEBUGGING & LOGGING
        // =====================================================================
        case WSE_OP_LOG_MESSAGE: {
            int64_t v1 = g_vm.eval_operand(stmt->operands[0], local_vars);
            int64_t v2 = g_vm.eval_operand(stmt->operands[1], local_vars);
            g_vm.log("[WSE-ModLog] Message: %lld, %lld\n", v1, v2);
            printf("\033[1;33m[WSE-ModLog]\033[0m Message: %lld, %lld\n", v1, v2);
            return 1;
        }

        case WSE_OP_TEST_OPCODE: {
            g_vm.log(">>> [WSE-Mac] Successfully executed CUSTOM TEST OPCODE 9999! <<<\n");
            printf("\033[1;32m>>> [WSE-Mac] Successfully executed CUSTOM TEST OPCODE 9999! <<<\033[0m\n");
            return 1;
        }

        default:
            return 0;
    }
}
