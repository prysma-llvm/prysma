#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"$SCRIPT_DIR/docker-cli.sh" rm -f prysma-python-debug 2>/dev/null || true

