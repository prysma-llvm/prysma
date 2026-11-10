#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET="$1"
if [ -z "$TARGET" ] || [ "${TARGET##*.}" != "py" ]; then
    TARGET="debug.py"
fi

"$SCRIPT_DIR/docker-cli.sh" rm -f prysma-python-debug 2>/dev/null || true
"$SCRIPT_DIR/docker-cli.sh" run -d --rm --name prysma-python-debug \
    -p 5678:5678 --user "$(id -u):$(id -g)" -e HOME=/tmp \
    -v "$SCRIPT_DIR":/workspace -w /workspace \
    prysma-debug python3 -m debugpy --listen 0.0.0.0:5678 --wait-for-client "$TARGET"

python3 -c "
import socket, time
for _ in range(60):
    try:
        s = socket.socket()
        s.connect(('127.0.0.1', 5678))
        s.close()
        break
    except Exception:
        time.sleep(0.1)
"

