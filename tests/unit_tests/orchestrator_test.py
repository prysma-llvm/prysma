import sys
import multiprocessing
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed
from typing import List

from unit_test import UnitTest, TestResult

def execute_test(compiler_path: Path, test_file: Path) -> TestResult:
    return UnitTest(compiler_path).run(test_file)

class TestLocator:
    def __init__(self, directories: List[Path]):
        self.directories = directories

    def get_all_test_files(self) -> List[Path]:
        valid_files = []
        for d in self.directories:
            if d.exists():
                for f in d.rglob("*.p"):
                    if f.is_file():
                        try:
                            # Only execute files that contain a main function
                            if "main" in f.read_text(encoding="utf-8"):
                                valid_files.append(f)
                        except Exception:
                            pass
        return sorted(valid_files)

class TestOrchestrator:
    def __init__(self, locator: TestLocator, compiler_path: Path):
        self.locator = locator
        self.compiler_path = compiler_path

    def run_tests(self):
        test_files = self.locator.get_all_test_files()
        
        if not test_files:
            sys.exit(0)

        if not self.compiler_path.exists():
            print(f"Compiler not found: {self.compiler_path}")
            sys.exit(1)

        fail_count = 0

        with ProcessPoolExecutor(max_workers=max(1, multiprocessing.cpu_count() - 1)) as executor:
            futures = [executor.submit(execute_test, self.compiler_path, f) for f in test_files]
            for future in as_completed(futures):
                result = future.result()
                if not result.success:
                    print(f"FAIL: {result.file_path.name}")
                    if result.stdout.strip(): print(result.stdout)
                    if result.stderr.strip(): print(result.stderr)
                    fail_count += 1

        if fail_count > 0:
            sys.exit(1)
        else:
            print(f"{len(test_files)} Prysma files successfully tested.")

if __name__ == "__main__":
    base_dir = Path(__file__).parent.resolve()
    gen_dir = base_dir / "build" / "prysma_code_generation"
    manual_dir = base_dir.parent / "prysma_code_tests"
    compiler = base_dir.parent.parent / "build" / "Prysma"

    locator = TestLocator([gen_dir, manual_dir])
    TestOrchestrator(locator, compiler).run_tests()
