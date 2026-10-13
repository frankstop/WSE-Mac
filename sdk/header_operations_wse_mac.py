################################################################################
# WARBAND SCRIPT ENHANCER (WSE) MAC - PYTHON MODULE SYSTEM HEADER
# For Mount & Blade: Warband macOS Native Modding (WarbandCraft & Conversions)
################################################################################

# ------------------------------------------------------------------------------
# Trigger Parameter Operations (Opcodes 2075 - 2080)
# ------------------------------------------------------------------------------
# (store_trigger_param_1, <destination>),
store_trigger_param_1 = 2075
# (store_trigger_param_2, <destination>),
store_trigger_param_2 = 2076
# (store_trigger_param_3, <destination>),
store_trigger_param_3 = 2077
# (store_trigger_param_4, <destination>),
store_trigger_param_4 = 2078
# (store_trigger_param_5, <destination>),
store_trigger_param_5 = 2079
# (store_trigger_param_6, <destination>),
store_trigger_param_6 = 2080

# ------------------------------------------------------------------------------
# Dynamic Arrays & Lists (Voxel Chunks, Crafting Inventories, Custom Entities)
# ------------------------------------------------------------------------------
# (array_create, <dest_array_id>, <initial_size>),
array_create    = 2850
# (array_set_val, <array_id>, <index>, <value>),
array_set_val   = 2851
# (array_get_val, <destination>, <array_id>, <index>),
array_get_val   = 2852
# (array_size, <destination>, <array_id>),
array_size      = 2853
# (array_clear, <array_id>),
array_clear     = 2854
# (array_free, <array_id>),
array_free      = 2855
# (array_push_back, <array_id>, <value>),
array_push_back = 2856

# ------------------------------------------------------------------------------
# Key-Value Dictionaries (Recipe Registries, Item Metadata, Block Types)
# ------------------------------------------------------------------------------
# (dict_create, <dest_dict_id>),
dict_create     = 2860
# (dict_set_val, <dict_id>, <key_int>, <value_int>),
dict_set_val    = 2861
# (dict_get_val, <destination>, <dict_id>, <key_int>),
dict_get_val    = 2862
# (dict_has_key, <dest_bool>, <dict_id>, <key_int>),
dict_has_key    = 2863
# (dict_clear, <dict_id>),
dict_clear      = 2864
# (dict_free, <dict_id>),
dict_free       = 2865

# ------------------------------------------------------------------------------
# Input & Controller Operations (Direct macOS SDL2 Hardware Queries)
# ------------------------------------------------------------------------------
# (key_is_down, <destination>, <sdl_scancode>),
key_is_down          = 2820
# (mouse_get_position, <dest_x>, <dest_y>),
mouse_get_position   = 2822
# (mouse_get_wheel_delta, <destination_delta>),  # Positive = Scroll Up, Negative = Scroll Down
mouse_get_wheel_delta = 2823
# (mouse_is_down, <destination>, <button_1_left_2_middle_3_right>),
mouse_is_down        = 2824

# ------------------------------------------------------------------------------
# High-Precision Timing & Animation Delta (Mining meters, smooth animations)
# ------------------------------------------------------------------------------
# (timer_get_time_ms, <destination_ms>),
timer_get_time_ms    = 2840
# (timer_get_delta_ms, <destination_delta_ms>),
timer_get_delta_ms   = 2841

# ------------------------------------------------------------------------------
# 3D Vector Math & Coordinate Grid Snapping (Voxel Placement & Raycasts)
# ------------------------------------------------------------------------------
# (position_snap_to_grid, <dest_val>, <coord_val>, <grid_size_cm>),
position_snap_to_grid        = 2817
# (position_get_distance_meters, <dest>, <x1>, <y1>, <z1>, <x2>, <y2>, <z2>),
position_get_distance_meters = 2811

# ------------------------------------------------------------------------------
# Mod File I/O Operations (Saving persistent world data, blueprints, logs)
# ------------------------------------------------------------------------------
# (file_open, <destination_handle>, <mode_0_read_1_write_2_append>),
file_open       = 2830
# (file_write_int, <file_handle>, <value>),
file_write_int  = 2832
# (file_read_int, <destination>, <file_handle>),
file_read_int   = 2833
# (file_close, <file_handle>),
file_close      = 2834

# ------------------------------------------------------------------------------
# Debug & Logging Operations
# ------------------------------------------------------------------------------
# (wse_log_message, <param1>, <param2>),
wse_log_message = 2890
# (wse_test_opcode),
wse_test_opcode = 9999
