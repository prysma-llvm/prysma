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
for CDB in "$SCRIPT_DIR/build/compile_commands.json" "$SCRIPT_DIR/tests/unit_tests/build/compile_commands.json"; do
    if [ -f "$CDB" ]; then
        if grep -q "/workspace" "$CDB" 2>/dev/null; then
            sed -i "s|/workspace|$SCRIPT_DIR|g" "$CDB"
        fi
        if grep -q "/usr/lib/llvm-18/include" "$CDB" 2>/dev/null; then
            sed -i "s|/usr/lib/llvm-18/include|$SCRIPT_DIR/.clangd-includes|g" "$CDB"
        fi
    fi
done

if [ -f "$SCRIPT_DIR/build/compile_commands.json" ] && [ -f "$SCRIPT_DIR/tests/unit_tests/build/compile_commands.json" ]; then
    python3 -c "
import json
try:
    with open('$SCRIPT_DIR/build/compile_commands.json') as f:
        main_cdb = json.load(f)
    with open('$SCRIPT_DIR/tests/unit_tests/build/compile_commands.json') as f:
        tests_cdb = json.load(f)
    files = {e['file'] for e in main_cdb}
    merged = main_cdb + [e for e in tests_cdb if e['file'] not in files]
    with open('$SCRIPT_DIR/build/compile_commands.json', 'w') as f:
        json.dump(merged, f, indent=2)
    with open('$SCRIPT_DIR/compile_commands.json', 'w') as f:
        json.dump(merged, f, indent=2)
except Exception:
    pass
" 2>/dev/null || true
fi

exit $EXIT_CODE
