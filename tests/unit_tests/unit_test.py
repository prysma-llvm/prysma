import subprocess
from pathlib import Path
from dataclasses import dataclass

@dataclass
class TestResult:
    file_path: Path
    success: bool
    stdout: str
    stderr: str

class UnitTest:
    def __init__(self, compiler_path: Path):
        self.compiler_path = compiler_path

    def run(self, test_file_path: Path) -> TestResult:
        try:
            result = subprocess.run(
                [str(self.compiler_path), str(test_file_path)],
                capture_output=True,
                text=True,
                timeout=10
            )
            return TestResult(test_file_path, result.returncode == 0, result.stdout, result.stderr)
        except subprocess.TimeoutExpired as e:
            return TestResult(test_file_path, False, getattr(e, 'stdout', '') or "", "Timeout")
        except Exception as e:
            return TestResult(test_file_path, False, "", str(e))
