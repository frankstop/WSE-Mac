#!/usr/bin/env python3
"""
Native macOS Python 3 Module System Compiler for Mount & Blade: Warband with WSE-Mac.
Compiles all module_*.py files and outputs directly into the target module folder.
"""

import sys
import os
import time
import subprocess

PROCESS_SCRIPTS = [
    "process_init",
    "process_global_variables",
    "process_strings",
    "process_skills",
    "process_music",
    "process_animations",
    "process_meshes",
    "process_sounds",
    "process_skins",
    "process_map_icons",
    "process_factions",
    "process_items",
    "process_scenes",
    "process_troops",
    "process_particle_sys",
    "process_scene_props",
    "process_tableau_materials",
    "process_presentations",
    "process_party_tmps",
    "process_parties",
    "process_quests",
    "process_info_pages",
    "process_scripts",
    "process_mission_tmps",
    "process_game_menus",
    "process_simple_triggers",
    "process_dialogs",
    "process_global_variables_unused",
    "process_postfx",
]

def compile_mod():
    start_time = time.time()
    print("=" * 60)
    print("   WARBAND WSE-MAC NATIVE MODULE COMPILER (Python 3)")
    print("=" * 60)

    sys.dont_write_bytecode = True

    for script in PROCESS_SCRIPTS:
        script_file = f"{script}.py"
        if not os.path.exists(script_file):
            continue

        print(f"[*] Processing: {script}...")
        res = subprocess.run([sys.executable, script_file], capture_output=True, text=True)
        if res.returncode != 0:
            print(f"\n[ERROR] Compilation failed in {script_file}:")
            print(res.stderr or res.stdout)
            sys.exit(1)
        elif res.stdout.strip():
            print(res.stdout.strip())

    elapsed = time.time() - start_time
    print("=" * 60)
    print(f"[SUCCESS] Module built successfully in {elapsed:.2f} seconds!")
    print("=" * 60)

if __name__ == "__main__":
    compile_mod()
