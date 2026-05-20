#!/bin/bash
set -e

BRANCH_NAME=$1
if [ -z "$BRANCH_NAME" ]; then
  echo "Error: Branch name not provided."
  exit 1
fi

echo "Cloning branch $BRANCH_NAME..."
git clone --branch "$BRANCH_NAME" https://github.com/prysma-llvm/prysma.git /prysma/prysma

echo "Running performance tests..."
python3 /prysma/prysma/tests/run_perf_tests.py

echo "Copying results..."
cp /prysma/prysma/perf_run_data.json /workspace/perf_run_data.json

echo "Cleaning up clone..."
rm -rf /prysma/prysma

echo "Done."
