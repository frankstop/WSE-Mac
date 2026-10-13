# Warband Script Enhancer (WSE) for macOS

**WSE-Mac** is a native, reverse-engineered engine extender for the 64-bit macOS build of *Mount & Blade: Warband*.

Instead of running the Windows version of the game through Wine or CrossOver, WSE-Mac hooks directly into the native 64-bit Mach-O binary (`Mount and Blade.app`) at runtime using macOS dynamic library interposition and inline machine-code trampolines.

---

## Directory Structure

```text
WSE_Mac/
├── libwse_mac.dylib           # Compiled 64-bit injection library (43 KB)
├── WSE_Launcher.command       # Double-clickable native macOS launcher
├── include/
│   ├── wse_core.h             # Bytecode statement structures, VM, Array, Dict & Timer interfaces
│   └── wse_operations.h       # Extended operation OpCodes
├── src/
│   ├── wse_core.cpp           # Operand evaluation, SDL2 event hooks, ASLR resolution & memory patcher
│   ├── wse_operations.cpp     # Extended C++ operation implementations
│   └── wse_trampoline.s       # Low-level x86_64 assembly register preservation bridge
└── sdk/
    ├── header_operations_wse_mac.py  # TaleWorlds Python Module System header
    └── build_module.py               # Native macOS Python 3 mod compiler
```

---

## Subsystems Implemented for Seamless Gameplay

### 1. Dynamic In-Memory Data Structures (Voxel Chunks & Crafting)
* **`array_create` / `array_set_val` / `array_get_val` / `array_push_back`**:
  Allows storing arbitrary 3D grid data, voxel coordinates, and custom inventory slots in RAM without abusing fixed troop/party slots.
* **`dict_create` / `dict_set_val` / `dict_get_val` / `dict_has_key`**:
  O(1) hash table lookup for crafting recipes, block traits (hardness, drops), and entity metadata.

### 2. Direct macOS Hardware Input & Hotbar Scrolling
* **`mouse_get_wheel_delta`**:
  Directly intercepts macOS SDL2 mouse wheel scroll events. Returning `+1` (scroll up) or `-1` (scroll down) allows cycling hotbar slots (items 1–9) smoothly without interfering with game camera zoom.
* **`mouse_is_down`**:
  Detects Left Click (mining / attacking), Middle Click (pick block), and Right Click (place block / interact).
* **`key_is_down`**:
  Queries arbitrary physical keys via SDL scancodes (e.g. holding Left Shift for sneaking/crouching).

### 3. High-Precision Timing & Fluid Animations
* **`timer_get_time_ms` / `timer_get_delta_ms`**:
  Sub-millisecond timers using steady clock counters. Essential for fluid block-breaking progress meters, cooldowns, and custom animations independent of mission tick rate.

### 4. 3D Grid Snapping & Vector Math
* **`position_snap_to_grid`**:
  Snaps raw 3D float coordinates directly to voxel cube boundaries (e.g., 100cm cube grid) in a single instruction.
* **`position_get_distance_meters`**:
  High-precision 3D Euclidean distance calculation between two points.

### 5. Persistent World File I/O
* **`file_open` / `file_write_int` / `file_read_int` / `file_close`**:
  Allows reading and writing custom world save files, block coordinates, and player blueprints directly to disk.

---

## Quick Reference: Available Operations

| Opcode | Operation Signature | Purpose |
| :--- | :--- | :--- |
| **2075-2080** | `(store_trigger_param_1..6, <dest>)` | Direct trigger parameter retrieval |
| **2850** | `(array_create, <dest_id>, <size>)` | Allocate dynamic memory array |
| **2851** | `(array_set_val, <arr_id>, <idx>, <val>)` | Set array value |
| **2852** | `(array_get_val, <dest>, <arr_id>, <idx>)` | Get array value |
| **2856** | `(array_push_back, <arr_id>, <val>)` | Append value to dynamic array |
| **2860** | `(dict_create, <dest_dict_id>)` | Allocate hash map dictionary |
| **2861** | `(dict_set_val, <dict_id>, <key>, <val>)` | Store key-value pair |
| **2862** | `(dict_get_val, <dest>, <dict_id>, <key>)` | Retrieve value by key |
| **2863** | `(dict_has_key, <dest_bool>, <dict_id>, <key>)` | Check if key exists (1 or 0) |
| **2820** | `(key_is_down, <dest>, <scancode>)` | Query physical keyboard state |
| **2822** | `(mouse_get_position, <dest_x>, <dest_y>)` | Query screen mouse coordinates |
| **2823** | `(mouse_get_wheel_delta, <dest_delta>)` | Read & consume mouse wheel scroll (+1 / -1) |
| **2824** | `(mouse_is_down, <dest>, <btn_1_2_3>)` | Check Left, Middle, or Right mouse click |
| **2840** | `(timer_get_time_ms, <dest_ms>)` | Current millisecond timestamp |
| **2841** | `(timer_get_delta_ms, <dest_delta_ms>)` | Milliseconds elapsed since last check |
| **2817** | `(position_snap_to_grid, <dest>, <coord>, <size>)` | Snap coordinate to voxel grid |
| **2811** | `(position_get_distance_meters, <dest>, <x1>, <y1>, <z1>, <x2>, <y2>, <z2>)` | 3D distance in meters |
| **2830** | `(file_open, <dest_handle>, <mode>)` | Open custom data file (0=read, 1=write, 2=append) |
| **2832** | `(file_write_int, <handle>, <val>)` | Write integer to file |
| **2833** | `(file_read_int, <dest>, <handle>)` | Read integer from file |
| **2834** | `(file_close, <handle>)` | Close file handle |
| **2890** | `(wse_log_message, <val1>, <val2>)` | Log debug message to `/tmp/wse_mac.log` |

---

## How to Mod

1. In your mod's module system:
   ```python
   from header_operations import *
   from header_operations_wse_mac import *
   ```
2. Compile directly on macOS:
   ```bash
   python3 /Users/frankvaldez/Desktop/WSE_Mac/sdk/build_module.py
   ```
3. Run the game with WSE-Mac:
   Double-click **`WSE_Launcher.command`** on your Desktop.
