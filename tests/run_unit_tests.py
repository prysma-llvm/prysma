import argparse
import sys
import os
import subprocess
from pathlib import Path

def main():
    parser = argparse.ArgumentParser(description="Prysma test suites")
    selection = parser.add_mutually_exclusive_group()
    selection.add_argument("--dev-env", action="store_true", help="Run only development environment integration tests")
    selection.add_argument("--existing", action="store_true", help="Run only the original five steps")
    args, dev_args = parser.parse_known_args()
    if args.existing and dev_args:
        parser.error("Development environment options cannot be used with --existing")
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    tests_dir = os.path.join(root_dir, "tests")
    unit_tests_dir = os.path.join(tests_dir, "unit_tests")

    total = 5 if args.existing else 6
    if not args.dev_env:
        print(f"[1/{total}] Building Prysma compiler (via debug.py with sanitizers)...")
        subprocess.run(["python3", "debug.py"], cwd=root_dir, check=True)

        print(f"[2/{total}] Configuring and building C++ unit tests (PrysmaTests)...")
        subprocess.run(["cmake", "-S", ".", "-B", "build"], cwd=unit_tests_dir, check=True)
        subprocess.run(["cmake", "--build", "build", "-j", str(max(1, os.cpu_count() or 1))], cwd=unit_tests_dir, check=True)

        print(f"[3/{total}] Running C++ unit tests (Catch2)...")
        test_exe = Path(unit_tests_dir) / "build" / "PrysmaTests"
        if test_exe.exists():
            try:
                subprocess.run([str(test_exe)], cwd=root_dir, check=True)
            except subprocess.CalledProcessError:
                print("C++ Unit tests failed!")
                sys.exit(1)
        else:
            print(f"Warning: Executable not found at {test_exe}")

        print(f"[4/{total}] Generating algorithmic integration tests (.p files)...")
        subprocess.run([sys.executable, "tests/unit_tests/generate_tests.py"], cwd=root_dir, check=True)

        print(f"[5/{total}] Running LLVM/Prysma integration test orchestrator...")
        try:
            subprocess.run([sys.executable, "tests/unit_tests/orchestrator_test.py"], cwd=root_dir, check=True)
        except subprocess.CalledProcessError:
            sys.exit(1)

    if not args.existing:
        print(f"[{1 if args.dev_env else 6}/{1 if args.dev_env else 6}] Running development environment integration tests...", flush=True)
        result = subprocess.run([sys.executable, "tests/environnement_dev_tests/orchestrator.py", *dev_args], cwd=root_dir)
        sys.exit(result.returncode)

if __name__ == "__main__":
    main()
