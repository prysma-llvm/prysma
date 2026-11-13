#!/bin/bash
# docker/environnement_dev/linux/docker-debug-lldb-tests.sh - LLDB Server starter for Unit Tests on Linux
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"

# Clean up any existing container and release port 1234
"$SCRIPT_DIR/docker-cli.sh" rm -f prysma-lldb-tests-debug prysma-lldb-debug 2>/dev/null || true

# Signal VS Code problem matcher that LLDB server is listening
echo "lldb-server ready on 1234"

# Launch LLDB server in container for unit tests
exec "$SCRIPT_DIR/docker-cli.sh" run --rm --name prysma-lldb-tests-debug \
    -p 1234:1234 --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
    -v "$ROOT_DIR":/workspace -w /workspace/tests/unit_tests \
    -e ASAN_OPTIONS=detect_leaks=0 \
    prysma-debug /usr/lib/llvm-18/bin/lldb-server-18 gdbserver 0.0.0.0:1234 -- /workspace/tests/unit_tests/build/PrysmaTests

