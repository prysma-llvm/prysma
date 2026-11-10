#!/bin/bash
# Wrapper transparent pour exécuter Docker que VS Code tourne en Flatpak ou en natif
if command -v flatpak-spawn >/dev/null 2>&1; then
    exec flatpak-spawn --host docker "$@"
else
    if command -v docker >/dev/null 2>&1; then
        exec docker "$@"
    elif [ -x /usr/bin/docker ]; then
        exec /usr/bin/docker "$@"
    else
        echo "Erreur: Docker est introuvable sur le système hôte." >&2
        exit 127
    fi
fi

