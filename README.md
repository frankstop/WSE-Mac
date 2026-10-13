# Warband Script Enhancer (WSE) for macOS

[![Platform](https://img.shields.io/badge/Platform-macOS%2064--bit%20(Mach--O)-brightgreen.svg)]()
[![Target](https://img.shields.io/badge/Target-Mount%20%26%20Blade%3A%20Warband%201.174-blue.svg)]()
[![Status](https://img.shields.io/badge/Status-Native%20Hook%20Active-success.svg)]()

**WSE-Mac** is a native, reverse-engineered engine extender for the 64-bit macOS build of *Mount & Blade: Warband*.

Instead of running the Windows version of the game through Wine, CrossOver, or virtual machines, WSE-Mac hooks directly into the native 64-bit Mach-O binary (`Mount and Blade.app`) at runtime using dynamic library interposition and inline machine-code trampolines.

---

## What is WSE (Warband Script Enhancer) For?

### The Core Problem in Mount & Blade Modding
*Mount & Blade: Warband* is celebrated for its total-conversion modding community, but the native game engine (originally developed by TaleWorlds in the late 2000s) has strict, hardcoded engine limitations:
1. **Rigid Scripting Architecture**: Vanilla scripting is interpreted bytecode executing a fixed set of ~500 operations. Any operation code not explicitly programmed into the engine crashes the game or triggers `SCRIPT ERROR ON OPCODE`.
2. **No Dynamic Memory**: Vanilla Warband provides no dynamic arrays, heap allocation, or hash tables. Modders wanting to store state are forced to abuse fixed troop, party, or scene prop "slots," creating severe memory bottlenecks.
3. **Limited Hardware Input**: The vanilla engine only listens to keys configured in its launcher menu. It cannot detect arbitrary physical keyboard keys, mouse scroll wheel deltas, or multi-button mouse clicks.
4. **No Direct Disk Access**: Vanilla scripts cannot read or write custom files to disk, making persistent world state, custom config files, or external data pipelines impossible.
5. **Tick-Rate Dependent Timing**: Vanilla triggers only fire at coarse mission ticks, making smooth real-time mechanics (mining animations, cooldown bars, raycast checks) jittery.

### What WSE Accomplishes
Originally developed on Windows by **cmpxchg8b** (WSE v1–v4) and later expanded by **K700** (WSE2), the **Warband Script Enhancer** fundamentally upgrades the game from a limited medieval battle simulator into a flexible, general-purpose RPG and sandbox game engine. 

WSE achieves this by:
* **Hooking the Bytecode Interpreter (`operation_Execute`)**: Intercepting unrecognized operations at the binary level and routing them into high-performance C++ execution tables.
* **Providing Dynamic In-Memory Data Structures**: Giving modders native $O(1)$ arrays and dictionaries for complex inventories, crafting matrices, and 3D voxel chunk allocation.
* **Direct Hardware Queries via SDL2**: Intercepting mouse scroll wheel deltas (essential for cycling 1–9 hotbars) and querying raw keyboard states.
* **Sub-Millisecond Delta Timing**: Enabling fluid, frame-rate-independent gameplay animations and interaction meters.
* **Persistent File I/O**: Allowing mods to save and load arbitrary world files and player data directly to the filesystem.

---

## Why WSE-Mac?

Historically, WSE only existed for Windows:
* WSE v1–v4 was a 32-bit Windows DLL injector (`WSELib.dll`) hardcoded to Windows PE virtual memory addresses.
* WSE2 is a closed-source Windows executable replacement (`mb_warband_wse2.exe`) compiled against Win32 and DirectX 9.

When TaleWorlds released their official **64-bit native macOS build** on Steam, Mac players and modders were left completely stranded: the Windows WSE injectors could not run on macOS, and the native 64-bit Mach-O executable had completely different calling conventions (System V AMD64 ABI vs. MSVC `__thiscall`), 64-bit struct padding, and stripped symbols.

**WSE-Mac** bridges this gap by providing the first native macOS runtime extension framework built specifically for the 64-bit Mach-O Warband executable.

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
  $O(1)$ hash table lookup for crafting recipes, block traits (hardness, drops), and entity metadata.

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

## API Reference: Extended Operations

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
| **9999** | `(wse_test_opcode)` | Built-in test verification opcode |

---

## Mod Development Workflow

### 1. Import WSE Headers
In your mod's module system files (`module_scripts.py`, `module_mission_templates.py`, etc.):
```python
from header_operations import *
from header_operations_wse_mac import *
```

### 2. Example: Mining & Hotbar Script
```python
# Hotbar Selection via Mouse Wheel:
(mouse_get_wheel_delta, ":scroll_delta"),
(try_begin),
  (neq, ":scroll_delta", 0),
  (val_add, "$current_hotbar_slot", ":scroll_delta"),
  (val_mod, "$current_hotbar_slot", 9),
(try_end),

# Block Mining via Left-Click:
(mouse_is_down, ":is_mining", 1),
(try_begin),
  (eq, ":is_mining", 1),
  (timer_get_delta_ms, ":dt_ms"),
  (val_add, "$block_break_progress", ":dt_ms"),
  (ge, "$block_break_progress", 1000), # 1000 ms to mine
  # Spawn dropped block item and play sound
  (assign, "$block_break_progress", 0),
(try_end),
```

### 3. Compile Directly on macOS
Run the included Python 3 compiler:
```bash
python3 /Users/frankvaldez/Desktop/WSE_Mac/sdk/build_module.py
```

### 4. Launch the Game with WSE-Mac
Double-click **`WSE_Launcher.command`** on your Desktop.

---

## Technical Architecture & Memory Offsets

Discovered from reverse engineering TaleWorlds' 64-bit Mach-O binary (`Mount and Blade`):

* **Interpreter Entry Point**: `0x100ab80b0` (`operation_Execute`)
* **Calling Convention**: System V AMD64 ABI:
  * `%rdi`: `const Statement* statement`
  * `%rsi`: `int64_t* local_variables`
  * `%rdx`: `void* script_context`
  * `%rcx`: `bool* condition_result`
* **Statement Memory Layout**:
  * `+0x00`: `int32_t opcode`
  * `+0x08`: `int64_t num_operands`
  * `+0x10 + i * 8`: `int64_t operand[i]`
* **Operand Tag Encoding** (High byte `raw_operand >> 56`):
  * `Tag 0`: Immediate integer constant
  * `Tag 1`: Global variable (`$var`) at `0x10145ce78 + slide`
  * `Tag 2`: System registers (`reg0`..`reg63`) at `0x10145d720 + slide`
  * `Tag 17` (`0x11`): Local variable (`:var`) in the `local_variables` array
* **Opcode Fallback Dispatcher**: `0x100ad5747` (patched with 14-byte 64-bit indirect jump to `wse_opcode_trampoline`)
* **Statement Completion Target**: `0x100adcd5d`
