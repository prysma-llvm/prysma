import os
import re
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from unit_test import UnitTest

MAIN = re.compile(r"(?m)^[ \t]*fn[ \t]+int32[ \t]+main[ \t]*\(")


class TestLocator:
    def __init__(self, directories):
        self.directories = directories

    def get_all_test_files(self):
        return [(root, path.relative_to(root))
                for root in self.directories
                for path in sorted(root.rglob("*.p"))
                if MAIN.search(path.read_text(encoding="utf-8"))]


class TestOrchestrator:
    def __init__(self, locator: TestLocator, compiler_path: Path):
        self.locator = locator
        self.compiler_path = compiler_path

    def run_tests(self):
        test_files = self.locator.get_all_test_files()
        if not test_files:
            print("No Prysma integration tests found.")
            return 1
        if not self.compiler_path.is_file():
            print(f"Compiler not found: {self.compiler_path}")
            return 1

        for root in self.locator.directories:
            if not any(source_root == root for source_root, _ in test_files):
                print(f"No Prysma integration tests found in {root}")
                return 1

        runner = UnitTest(self.compiler_path)
        failures = 0
        with ThreadPoolExecutor(max_workers=min(8, os.cpu_count() or 1)) as executor:
            for result in executor.map(lambda test: runner.run(*test), test_files):
                if not result.success:
                    failures += 1
                    if result.stdout.strip():
                        print(result.stdout.rstrip())
                    if result.stderr.strip():
                        print(result.stderr.rstrip())

        print(f"{len(test_files) - failures}/{len(test_files)} Prysma files successfully tested.")
        return int(failures > 0)


if __name__ == "__main__":
    base_dir = Path(__file__).parent.resolve()
    gen_dir = base_dir / "build" / "prysma_code_generation"
    compiler = base_dir.parent.parent / "build" / "Prysma"

    locator = TestLocator([gen_dir])
    sys.exit(TestOrchestrator(locator, compiler).run_tests())
