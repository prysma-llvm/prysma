import os
import subprocess
import shutil

## This is where we manage project compilation.
## It compiles the project and discovers performance tests.
## PS: you only need to add a new file.cpp with a TEST_CASE to add it to the framework.
## Example: TEST_CASE("Lexer DFA - Numeric Literals", "[lexer][pmu]")

class ProjectBuilder:
    
    def __init__(self, root_dir):
        self.root_dir = root_dir
        self.perf_dir = os.path.join(root_dir, "tests", "perf_tests")
        self.build_dir = os.path.join(self.perf_dir, "build")
        self.exe_path = os.path.join(self.build_dir, "PrysmaPerfTests")

    def build_all(self):
        self._build_compiler()
        self._configure_tests()
        self._build_tests()
        self._verify_executable()
        return self.exe_path

    def discover_tests(self):
        result = subprocess.run(
            [self.exe_path, "--list-test-names-only"],
            cwd=self.root_dir, capture_output=True, text=True, check=False
        )
        return [line.strip() for line in result.stdout.splitlines() if line.strip()]

    def _build_compiler(self):
        print("[1/5] Compiling Prysma...")
        subprocess.run(["python3", "build.py"], cwd=self.root_dir, check=True)

    def _configure_tests(self):
        print("[2/5] CMake configuration of benchmarks...")
        if os.path.exists(self.build_dir):
            shutil.rmtree(self.build_dir)
        subprocess.run(
            ["cmake", "-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"],
            cwd=self.perf_dir, check=True
        )

    def _build_tests(self):
        print("[3/5] Compiling benchmarks...")
        cores = str(max(1, os.cpu_count() or 1))
        subprocess.run(["cmake", "--build", "build", "-j", cores], cwd=self.perf_dir, check=True)

    def _verify_executable(self):
        if not os.path.isfile(self.exe_path):
            raise FileNotFoundError(f"Executable not found: {self.exe_path}")
