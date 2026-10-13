#ifndef WSE_OPERATIONS_H
#define WSE_OPERATIONS_H

// =============================================================================
// WARBAND SCRIPT ENHANCER (WSE) MAC - EXPANDED OPERATION OPCODES
// =============================================================================

// Trigger Parameters (Opcodes 2075 - 2085)
#define WSE_OP_STORE_TRIGGER_PARAM_1           2075
#define WSE_OP_STORE_TRIGGER_PARAM_2           2076
#define WSE_OP_STORE_TRIGGER_PARAM_3           2077
#define WSE_OP_STORE_TRIGGER_PARAM_4           2078
#define WSE_OP_STORE_TRIGGER_PARAM_5           2079
#define WSE_OP_STORE_TRIGGER_PARAM_6           2080

// Dynamic Arrays & Lists (Voxel Chunks, Crafting Inventories, Custom Entities)
#define WSE_OP_ARRAY_CREATE                    2850
#define WSE_OP_ARRAY_SET_VAL                   2851
#define WSE_OP_ARRAY_GET_VAL                   2852
#define WSE_OP_ARRAY_SIZE                      2853
#define WSE_OP_ARRAY_CLEAR                     2854
#define WSE_OP_ARRAY_FREE                      2855
#define WSE_OP_ARRAY_PUSH_BACK                 2856

// Key-Value Dictionaries (Item Properties, Recipe Registries, Block Definitions)
#define WSE_OP_DICT_CREATE                     2860
#define WSE_OP_DICT_SET_VAL                    2861
#define WSE_OP_DICT_GET_VAL                    2862
#define WSE_OP_DICT_HAS_KEY                    2863
#define WSE_OP_DICT_CLEAR                      2864
#define WSE_OP_DICT_FREE                       2865

// Input & Controller Operations (Queried directly via SDL2 on macOS)
#define WSE_OP_KEY_IS_DOWN                     2820
#define WSE_OP_KEY_CLICKED                     2821
#define WSE_OP_MOUSE_GET_POSITION              2822
#define WSE_OP_MOUSE_GET_WHEEL_DELTA           2823  // Hotbar scrolling (mouse wheel up/down)
#define WSE_OP_MOUSE_IS_DOWN                   2824  // Left/Right/Middle mouse button

// High-Precision Timing & Animation Delta (Mining meters, smooth animations)
#define WSE_OP_TIMER_GET_TIME_MS               2840  // Millisecond timestamp
#define WSE_OP_TIMER_GET_DELTA_MS              2841  // Milliseconds since last call

// 3D Vector Math & Coordinate Grid Snapping (Voxel Placement & Raycasting)
#define WSE_OP_POSITION_GET_DISTANCE_METERS    2811
#define WSE_OP_POSITION_ROTATE_X               2812
#define WSE_OP_POSITION_ROTATE_Y               2813
#define WSE_OP_POSITION_ROTATE_Z               2814
#define WSE_OP_POSITION_GET_SCALE_VECTOR       2815
#define WSE_OP_POSITION_SET_SCALE_VECTOR       2816
#define WSE_OP_POSITION_SNAP_TO_GRID           2817  // Snaps coordinates to voxel grid (e.g. 100cm)

// String Manipulation & Recipe Parsing
#define WSE_OP_STR_EQUALS                      2870
#define WSE_OP_STR_CONTAINS                    2871
#define WSE_OP_STR_TO_LOWER                    2872
#define WSE_OP_STR_TO_UPPER                    2873
#define WSE_OP_STR_LENGTH                      2874

// Mod File I/O Operations (Persistent saving, custom world files, blueprints)
#define WSE_OP_FILE_OPEN                       2830
#define WSE_OP_FILE_WRITE_STRING               2831
#define WSE_OP_FILE_WRITE_INT                  2832
#define WSE_OP_FILE_READ_INT                   2833
#define WSE_OP_FILE_CLOSE                      2834

// Custom Debug & Logging
#define WSE_OP_LOG_MESSAGE                     2890
#define WSE_OP_TEST_OPCODE                     9999

#endif // WSE_OPERATIONS_H
