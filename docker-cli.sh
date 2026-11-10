#!/bin/bash
# Wrapper transparent pour exécuter Docker que VS Code tourne en Flatpak ou en natif
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if command -v flatpak-spawn >/dev/null 2>&1; then
    flatpak-spawn --host docker "$@"
    EXIT_CODE=$?
else
    if command -v docker >/dev/null 2>&1; then
        docker "$@"
        EXIT_CODE=$?
    elif [ -x /usr/bin/docker ]; then
        /usr/bin/docker "$@"
        EXIT_CODE=$?
    else
        echo "Erreur: Docker est introuvable sur le système hôte." >&2
        exit 127
    fi
fi

# Mettre à jour compile_commands.json pour Clangd sur l'hôte si généré dans Docker
if [ -f "$SCRIPT_DIR/build/compile_commands.json" ]; then
    if grep -q "/workspace" "$SCRIPT_DIR/build/compile_commands.json" 2>/dev/null; then
        sed -i "s|/workspace|$SCRIPT_DIR|g" "$SCRIPT_DIR/build/compile_commands.json"
    fi
    if grep -q "/usr/lib/llvm-18/include" "$SCRIPT_DIR/build/compile_commands.json" 2>/dev/null; then
        sed -i "s|/usr/lib/llvm-18/include|$SCRIPT_DIR/.clangd-includes|g" "$SCRIPT_DIR/build/compile_commands.json"
    fi
fi

exit $EXIT_CODE
