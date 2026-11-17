#!/bin/bash
# Docker LLDB server for Prysma and its unit tests.
source "$(dirname "${BASH_SOURCE[0]}")/docker-debug-common.sh"
debug_name lldb
if [ "${1:-}" = "--stop" ]; then
    stop_debug
    exit $?
fi
WORK_DIR=/workspace
TARGET_ARGS=()
if [ "${1:-}" = "--tests" ] || [ "${1:-}" = "tests" ]; then
    BIN_PATH="$ROOT_DIR/tests/unit_tests/build/PrysmaTests"
    CONTAINER_BIN=/workspace/tests/unit_tests/build/PrysmaTests
    WORK_DIR=/workspace/tests/unit_tests
else
    BIN_PATH="$ROOT_DIR/build/Prysma"
    CONTAINER_BIN=/workspace/build/Prysma
    TARGET="${1:-}"
    if [ -z "$TARGET" ] || [ "${TARGET##*.}" != "p" ]; then
        TARGET="tests/unit_tests/build/prysma_code_generation/scenarios/project_test_multi_thread_edge_case_file/test_depth/main.p"
        if [ ! -f "$ROOT_DIR/$TARGET" ]; then
            docker_command run --rm --pull=never --user "$(id -u):$(id -g)" \
                -v "$ROOT_DIR:/workspace" -w /workspace "$IMAGE" python3 tests/unit_tests/generate_tests.py
        fi
    fi
    TARGET_ARGS+=("$(target_path "$TARGET")")
fi
if [ ! -x "$BIN_PATH" ]; then
    echo "ERROR: Target binary is missing or not executable: $BIN_PATH" >&2
    exit 1
fi
PORT="${PRYSMA_DEBUG_PORT:-1234}"
start_debug "$PORT" "lldb-server ready" \
    --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
    -w "$WORK_DIR" -e ASAN_OPTIONS=detect_leaks=0 \
    "$IMAGE" /usr/lib/llvm-18/bin/lldb-server-18 gdbserver "0.0.0.0:$PORT" -- "$CONTAINER_BIN" "${TARGET_ARGS[@]}"
