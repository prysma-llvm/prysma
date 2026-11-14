#!/bin/bash
# Docker debugpy server for the current workspace.
source "$(dirname "${BASH_SOURCE[0]}")/docker-debug-common.sh"
debug_name python
if [ "${1:-}" = "--stop" ]; then
    stop_debug
    exit $?
fi
TARGET="${1:-debug.py}"
if [ "${TARGET##*.}" != "py" ]; then
    TARGET="debug.py"
fi
TARGET="$(target_path "$TARGET")"
PORT="${PRYSMA_DEBUG_PORT:-5678}"
start_debug "$PORT" "debugpy server ready" \
    --user "$(id -u):$(id -g)" -e HOME=/tmp -e PYTHONDONTWRITEBYTECODE=1 \
    -w /workspace "$IMAGE" python3 -m debugpy --listen "0.0.0.0:$PORT" --wait-for-client "$TARGET"
