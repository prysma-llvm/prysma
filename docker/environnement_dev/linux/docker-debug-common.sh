#!/bin/bash
# Shared lifecycle for the two Docker debug servers.
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"
IMAGE="${PRYSMA_DEBUG_IMAGE:-prysma-debug}"
START_TIMEOUT="${PRYSMA_DEBUG_START_TIMEOUT:-15}"
COMMAND_TIMEOUT="${PRYSMA_DOCKER_TIMEOUT:-30}"
CONTAINER_ID=""
LOG_PID=""

docker_command() {
    PRYSMA_DOCKER_RAW=1 timeout --foreground "$COMMAND_TIMEOUT" "$SCRIPT_DIR/docker-cli.sh" "$@"
}

debug_name() {
    local project
    project="$(printf '%s' "$ROOT_DIR" | sha256sum)"
    CONTAINER_NAME="prysma-$1-${project:0:16}"
}

stop_debug() {
    local details container owner output status remaining
    # Read identity and ownership together; the launcher may remove the container at any time.
    details="$(docker_command ps -a --no-trunc --filter "name=^/${CONTAINER_NAME}$" \
        --format '{{.ID}} {{.Label "prysma.workspace"}}')" || return $?
    [ -n "$details" ] || return 0
    container="${details%% *}"
    owner="${details#* }"
    if [ "$owner" != "$ROOT_DIR" ]; then
        echo "ERROR: Refusing to stop a container owned by another workspace." >&2
        return 1
    fi
    if output="$(docker_command stop --time 2 "$container" 2>&1)"; then
        return 0
    else
        status=$?
    fi
    # A concurrent cleanup is success, but a daemon/stop error must still fail.
    remaining="$(docker_command ps -aq --no-trunc --filter "id=$container")" || return $?
    [ -n "$remaining" ] || return 0
    printf '%s\n' "$output" >&2
    return "$status"
}

target_path() {
    python3 - "$ROOT_DIR" "$1" <<'PY'
import pathlib
import sys
root = pathlib.Path(sys.argv[1]).resolve()
target = sys.argv[2]
for prefix in ("/workspace/", "workspace/"):
    if target.startswith(prefix):
        target = target[len(prefix):]
        break
path = (root / target).resolve()
try:
    relative = path.relative_to(root)
except ValueError:
    sys.exit("ERROR: Target must be inside the workspace")
if not path.is_file():
    sys.exit(f"ERROR: Target does not exist: {path}")
print("/workspace/" + relative.as_posix())
PY
}

cleanup_debug() {
    local status=$?
    trap - EXIT INT TERM
    if [ -n "$LOG_PID" ]; then
        kill "$LOG_PID" 2>/dev/null || true
        wait "$LOG_PID" 2>/dev/null || true
    fi
    if [ -n "$CONTAINER_ID" ]; then
        docker_command rm -f "$CONTAINER_ID" >/dev/null 2>&1 || true
    fi
    exit "$status"
}

start_debug() {
    local port="$1" message="$2"
    shift 2
    trap cleanup_debug EXIT
    trap 'exit 130' INT
    trap 'exit 143' TERM
    local labels=(--label "prysma.workspace=$ROOT_DIR")
    if [ -n "${PRYSMA_TEST_RUN_ID:-}" ]; then
        labels+=(--label "prysma.test=$PRYSMA_TEST_RUN_ID")
    fi
    # Docker atomically rejects duplicate names and occupied ports. Never remove an existing session.
    echo "${message% ready} starting"
    CONTAINER_ID="$(docker_command create --pull=never --name "$CONTAINER_NAME" "${labels[@]}" \
        -p "127.0.0.1:$port:$port" -v "$ROOT_DIR:/workspace" "$@")"
    docker_command start "$CONTAINER_ID" >/dev/null
    # Inspect LISTEN inside this container; a TCP probe would consume LLDB's connection.
    if ! docker_command exec "$CONTAINER_ID" python3 -c '
import pathlib, sys, time
port, seconds = int(sys.argv[1]), float(sys.argv[2])
deadline = time.monotonic() + seconds
while time.monotonic() < deadline:
    for table in ("/proc/net/tcp", "/proc/net/tcp6"):
        for row in pathlib.Path(table).read_text().splitlines()[1:]:
            fields = row.split()
            if int(fields[1].split(":")[1], 16) == port and fields[3] == "0A":
                sys.exit(0)
    time.sleep(0.1)
sys.exit("ERROR: Debug server readiness deadline exceeded")
' "$port" "$START_TIMEOUT"; then
        docker_command logs "$CONTAINER_ID" >&2 || true
        return 1
    fi
    echo "$message on $port"
    # No timeout while the developer is paused at a breakpoint.
    PRYSMA_DOCKER_RAW=1 "$SCRIPT_DIR/docker-cli.sh" logs -f "$CONTAINER_ID" &
    LOG_PID=$!
    wait "$LOG_PID" || true
    LOG_PID=""
    local status
    status="$(docker_command wait "$CONTAINER_ID")"
    return "$status"
}
