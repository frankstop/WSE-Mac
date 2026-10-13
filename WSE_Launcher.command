#!/bin/bash
# ==============================================================================
# WARBAND SCRIPT ENHANCER (WSE) MAC - ONE-CLICK LAUNCHER
# ==============================================================================

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
GAME_DIR="/Users/frankvaldez/Library/Application Support/Steam/steamapps/common/MountBlade Warband"
GAME_EXEC="$GAME_DIR/Mount and Blade.app/Contents/MacOS/Mount and Blade"
WSE_DYLIB="$DIR/libwse_mac.dylib"

echo "=========================================================="
echo "      WARBAND SCRIPT ENHANCER (WSE) MAC LAUNCHER         "
echo "=========================================================="

if [ ! -f "$WSE_DYLIB" ]; then
    echo "[!] Compiling libwse_mac.dylib..."
    clang++ -arch x86_64 -dynamiclib -std=c++17 -O2 -I"$DIR/include" -undefined dynamic_lookup \
        "$DIR/src/wse_core.cpp" "$DIR/src/wse_operations.cpp" "$DIR/src/wse_trampoline.s" \
        -o "$WSE_DYLIB"
fi

if [ ! -f "$GAME_EXEC" ]; then
    echo "[ERROR] Warband game executable not found at:"
    echo "  $GAME_EXEC"
    read -p "Press Enter to exit..."
    exit 1
fi

echo "[*] Injecting WSE Dynamic Library: $WSE_DYLIB"
echo "[*] Starting Mount & Blade: Warband natively on macOS..."
echo "----------------------------------------------------------"

# Launch with DYLD injection
DYLD_INSERT_LIBRARIES="$WSE_DYLIB" "$GAME_EXEC" "$@"

echo "----------------------------------------------------------"
echo "[*] Game session closed."
