import sys
import os
import subprocess
from pathlib import Path

def main():
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    tests_dir = os.path.join(root_dir, "tests")
    unit_tests_dir = os.path.join(tests_dir, "unit_tests")

    print("[1/5] Building Prysma compiler (via debug.py with sanitizers)...")
    subprocess.run(["python3", "build.py"], cwd=root_dir, check=True)

    print("[2/5] Configuring and building C++ unit tests (PrysmaTests)...")
    subprocess.run(["cmake", "-S", ".", "-B", "build"], cwd=unit_tests_dir, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.run(["cmake", "--build", "build", "-j", str(max(1, os.cpu_count() or 1))], cwd=unit_tests_dir, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    print("[3/5] Running C++ unit tests (Catch2)...")
    test_exe = Path(unit_tests_dir) / "build" / "PrysmaTests"
    if test_exe.exists():
        try:
            subprocess.run([str(test_exe)], cwd=root_dir, check=True)
        except subprocess.CalledProcessError:
            print("C++ Unit tests failed!")
            sys.exit(1)
    else:
        print(f"Warning: Executable not found at {test_exe}")

    print("[4/5] Generating algorithmic integration tests (.p files)...")
    subprocess.run(["python3", "tests/unit_tests/generate_tests.py"], cwd=root_dir, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    
    print("[5/5] Running LLVM/Prysma integration test orchestrator...")
    try:
        subprocess.run(["python3", "tests/unit_tests/orchestrator_test.py"], cwd=root_dir, check=True)
    except subprocess.CalledProcessError:
        sys.exit(1)

if __name__ == "__main__":
    main()
