#!/bin/bash
# Wrapper transparent pour exécuter Docker que VS Code tourne en Flatpak ou en natif
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../../.." && pwd)"
IMAGE="${PRYSMA_DEBUG_IMAGE:-prysma-debug}"

run_docker() {
    if [ -f /.flatpak-info ] && command -v flatpak-spawn >/dev/null 2>&1; then
        flatpak-spawn --host docker "$@"
    elif command -v docker >/dev/null 2>&1; then
        docker "$@"
    elif [ -x /usr/bin/docker ]; then
        /usr/bin/docker "$@"
    else
        echo "Erreur: Docker est introuvable sur le système hôte." >&2
        return 127
    fi
}

# Debug lifecycle commands must not populate caches or rewrite source databases.
if [ "${PRYSMA_DOCKER_RAW:-0}" = "1" ]; then
    run_docker "$@"
    exit $?
fi

# Ensure persistent ccache volume exists with write permissions BEFORE running commands
if [[ " $* " == *" prysma-ccache:/tmp/.ccache "* ]] && ! run_docker volume inspect prysma-ccache >/dev/null 2>&1; then
    run_docker volume create prysma-ccache >/dev/null 2>&1 || true
    run_docker run --rm -v prysma-ccache:/tmp/.ccache "$IMAGE" chmod 777 /tmp/.ccache >/dev/null 2>&1 || true
fi

# Ensure .clangd-includes is populated for host Clangd indexing
if [ "${1:-}" = "run" ] && [ "${PRYSMA_DOCKER_RAW:-0}" != "1" ] && [ ! -f "$ROOT_DIR/.clangd-includes/llvm/Support/TargetSelect.h" ]; then
    TEMP_C=$(run_docker create "$IMAGE" 2>/dev/null)
    if [ -n "$TEMP_C" ]; then
        mkdir -p "$ROOT_DIR/.clangd-includes"
        # Copy the real header directories (avoiding broken relative symlinks from /usr/lib)
        run_docker cp "$TEMP_C:/usr/include/llvm-18/llvm/." "$ROOT_DIR/.clangd-includes/llvm" 2>/dev/null
        run_docker cp "$TEMP_C:/usr/include/llvm-c-18/llvm-c/." "$ROOT_DIR/.clangd-includes/llvm-c" 2>/dev/null
        run_docker rm -f "$TEMP_C" >/dev/null 2>&1
        # Create llvm-18 and llvm-c-18 subdirectories for includes like <llvm-18/llvm/...>
        mkdir -p "$ROOT_DIR/.clangd-includes/llvm-18" "$ROOT_DIR/.clangd-includes/llvm-c-18"
        cp -r "$ROOT_DIR/.clangd-includes/llvm" "$ROOT_DIR/.clangd-includes/llvm-18/llvm" 2>/dev/null
        cp -r "$ROOT_DIR/.clangd-includes/llvm-c" "$ROOT_DIR/.clangd-includes/llvm-c-18/llvm-c" 2>/dev/null
    fi
fi

# Execute requested docker command
run_docker "$@"
EXIT_CODE=$?

# Adapt JSON structurally so spaces, quotes and Unicode paths remain valid.
if [ "$EXIT_CODE" -eq 0 ] && [ "${1:-}" = "run" ]; then
    python3 "$SCRIPT_DIR/../compile_commands.py" "$ROOT_DIR" || exit $?
fi
exit "$EXIT_CODE"
