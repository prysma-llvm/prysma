#!/bin/bash
# docker/environnement_dev/linux/docker-debug-lldb.sh - LLDB Server starter for Linux (Prysma & Unit Tests)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# Clean up any existing container and release port 1234
"$SCRIPT_DIR/docker-cli.sh" rm -f prysma-lldb-debug prysma-lldb-tests-debug 2>/dev/null || true

# Signal VS Code problem matcher that LLDB server is listening
echo "lldb-server ready on 1234"

if [ "$1" = "--tests" ] || [ "$1" = "tests" ]; then
    # Launch LLDB server for unit tests
    exec "$SCRIPT_DIR/docker-cli.sh" run --rm --name prysma-lldb-tests-debug \
        -p 1234:1234 --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
        -v "$ROOT_DIR":/workspace -w /workspace/tests/unit_tests \
        -e ASAN_OPTIONS=detect_leaks=0 \
        prysma-debug /usr/lib/llvm-18/bin/lldb-server-18 gdbserver 0.0.0.0:1234 -- /workspace/tests/unit_tests/build/PrysmaTests
fi

TARGET="$1"
if [ -z "$TARGET" ] || [ "${TARGET##*.}" != "p" ]; then
    TARGET="tests/prysma_code_tests/project_test_multi_thread_edge_case_file/test_depth/main.p"
fi

# Strip redundant /workspace prefix if present
if [[ "$TARGET" =~ ^/workspace/(.*)$ ]]; then
    TARGET="${BASH_REMATCH[1]}"
elif [[ "$TARGET" =~ ^workspace/(.*)$ ]]; then
    TARGET="${BASH_REMATCH[1]}"
fi

# Launch LLDB server in container for Prysma compiler
exec "$SCRIPT_DIR/docker-cli.sh" run --rm --name prysma-lldb-debug \
    -p 1234:1234 --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
    -v "$ROOT_DIR":/workspace -w /workspace \
    -e ASAN_OPTIONS=detect_leaks=0 \
    prysma-debug /usr/lib/llvm-18/bin/lldb-server-18 gdbserver 0.0.0.0:1234 -- /workspace/build/Prysma "/workspace/$TARGET"
