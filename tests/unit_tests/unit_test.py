import os
import subprocess
from dataclasses import dataclass
from pathlib import Path
from tempfile import TemporaryDirectory

from checkpoints import Checkpoint, InvalidExpectation, compare_trace, format_failure, prepare_sources


@dataclass
class TestResult:
    file_path: Path
    success: bool
    stdout: str = ""
    stderr: str = ""


class UnitTest:
    def __init__(self, compiler_path: Path):
        self.compiler_path = compiler_path.resolve()

    def run(self, source_root: Path, entry: Path) -> TestResult:
        try:
            # Prysma writes IR, runtime objects and binaries next to itself.
            # Each compile needs its own directory, including projects named main.p.
            with TemporaryDirectory(prefix="prysma-test-", dir=self.compiler_path.parent) as directory:
                workspace = Path(directory)
                build = workspace / "build"
                build.mkdir()
                compiler = build / self.compiler_path.name
                os.link(self.compiler_path, compiler)
                (workspace / "src").symlink_to(self.compiler_path.parent.parent / "src", target_is_directory=True)
                sources = workspace / "sources"
                points, expected, compile_error, qualified = prepare_sources(source_root, entry, sources)
                return self.compile_and_run(compiler, sources, entry, points, expected, compile_error, qualified)
        except InvalidExpectation as error:
            report = format_failure(error.point, "valid test assertion", error.message, "INVALID_EXPECTATION")
        except (OSError, ValueError) as error:
            point = Checkpoint(entry.as_posix(), None, None, "")
            report = format_failure(point, "test source and tools available", str(error), "TEST_SETUP_ERROR")
        return TestResult(entry, False, stderr=report)

    def compile_and_run(self, compiler, sources, entry, points, expected, compile_error, qualified):
        entry_point = Checkpoint(entry.as_posix(), None, None, "")
        try:
            compilation = subprocess.run(
                [str(compiler), str(sources / entry)], capture_output=True,
                text=True, timeout=30, cwd=sources,
            )
        except subprocess.TimeoutExpired:
            return TestResult(entry, False, stderr=format_failure(
                entry_point, "compilation within 30 seconds", "timeout", "COMPILE_TIMEOUT",
            ))

        diagnostics = compilation.stderr.replace(f"{sources}/", "").replace(f"{compiler.parent.parent}/", "")
        binary = compiler.parent / entry.stem
        if compile_error:
            point, pattern = compile_error
            if compilation.returncode == 1 and pattern.search(diagnostics):
                return TestResult(entry, True)
            actual = "compilation succeeded" if compilation.returncode == 0 and binary.is_file() else diagnostics.strip()
            actual = actual or f"compiler exit status {compilation.returncode}"
            return TestResult(entry, False, compilation.stdout, format_failure(
                point, f"compiler rejection matching /{pattern.pattern}/", actual, "COMPILE_ERROR_MISMATCH",
            ))

        if compilation.returncode != 0 or not binary.is_file():
            reason = "Compiler produced no executable"
            if compilation.returncode:
                reason = f"Compiler exited with status {compilation.returncode}"
            report = format_failure(entry_point, "executable produced", reason, "COMPILATION_FAILED")
            return TestResult(entry, False, compilation.stdout, f"{report}\n{diagnostics}")

        try:
            execution = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10, cwd=sources)
        except subprocess.TimeoutExpired as error:
            stdout = (error.stdout or b"").decode(errors="replace")
            divergence = compare_trace(stdout, expected, points, entry.as_posix(), qualified)
            report = divergence or format_failure(
                entry_point, "execution within 10 seconds", "timeout", "EXECUTION_TIMEOUT",
            )
            return TestResult(entry, False, stderr=report)

        divergence = compare_trace(execution.stdout, expected, points, entry.as_posix(), qualified)
        if divergence or execution.returncode:
            report = divergence or format_failure(
                entry_point, "exit status 0", f"exit status {execution.returncode}", "EXECUTION_FAILED",
            )
            error = "\n".join(filter(None, (report, execution.stderr.rstrip())))
            return TestResult(entry, False, stderr=error)
        return TestResult(entry, True)
