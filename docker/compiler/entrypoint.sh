#!/bin/bash
set -euo pipefail

BRANCH_NAME=${1:-}
if [ -z "$BRANCH_NAME" ]; then
  echo "Error: Branch name not provided."
  exit 1
fi

# Use a directory owned by this run; never remove an existing developer checkout.
RUN_DIR=$(mktemp -d /prysma/prysma-bench.XXXXXX)
cleanup() {
  echo "Cleaning up clone..."
  rm -rf -- "$RUN_DIR"
}
trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

echo "Cloning branch $BRANCH_NAME..."
git clone --branch "$BRANCH_NAME" -- https://github.com/prysma-llvm/prysma.git "$RUN_DIR/repo"

echo "Running performance tests..."
python3 "$RUN_DIR/repo/tests/run_perf_tests.py"

echo "Copying results..."
cp "$RUN_DIR/repo/perf_run_data.json" /workspace/perf_run_data.json

echo "Done."
