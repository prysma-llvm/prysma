import os
import shutil
import subprocess
import sys
import time
import uuid
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).resolve().parent))


def pytest_configure(config):
    for name in ("docker", "dap", "linux", "windows", "vscode"):
        config.addinivalue_line("markers", f"{name}: development environment test family")


def pytest_collection_modifyitems(items):
    for item in items:
        if "linux" in item.keywords and sys.platform != "linux":
            item.add_marker(pytest.mark.skip(reason="Requires Linux or WSL"))
        if "windows" in item.keywords and sys.platform != "win32":
            item.add_marker(pytest.mark.skip(reason="Requires native Windows"))


@pytest.fixture(scope="session")
def repo_root():
    return REPO_ROOT


@pytest.fixture
def workspace(tmp_path):
    root = tmp_path / "prysma espace é"
    root.mkdir()
    shutil.copytree(REPO_ROOT / "docker" / "environnement_dev", root / "docker" / "environnement_dev")
    shutil.copytree(REPO_ROOT / "tests" / "environnement_dev_tests" / "fixtures", root / "fixtures")
    return root


class Docker:
    def __init__(self):
        self.image = os.environ.get("PRYSMA_TEST_IMAGE", "prysma-debug")
        self.run_id = os.environ.get("PRYSMA_TEST_RUN_ID", uuid.uuid4().hex)
        self.containers = []
        self.volumes = []

    def command(self, *args, check=True, timeout=None):
        started = time.monotonic()
        result = subprocess.run(["docker", *map(str, args)], capture_output=True, text=True,
                                encoding="utf-8", errors="replace",
                                timeout=timeout or float(os.environ.get("PRYSMA_TEST_COMMAND_TIMEOUT", "90")))
        print(f"docker {args}: exit={result.returncode}, duration={time.monotonic() - started:.2f}s")
        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print(result.stderr)
        if check:
            assert result.returncode == 0, f"docker {args}: {result.stderr}"
        return result

    def run(self, *command, options=(), detach=False, check=True):
        name = f"prysma-test-{uuid.uuid4().hex}"
        self.containers.append(name)
        mode = ["-d"] if detach else ["--rm"]
        result = self.command("run", *mode, "--name", name,
                              "--label", f"prysma.test={self.run_id}", *options,
                              self.image, *command, check=check)
        if not detach:
            self.containers.remove(name)
        return name if detach else result

    def volume(self):
        name = f"prysma-test-{uuid.uuid4().hex}"
        self.volumes.append(name)
        self.command("volume", "create", "--label", f"prysma.test={self.run_id}", name)
        return name

    def close(self):
        for name in reversed(self.containers):
            try:
                self.command("logs", name, check=False, timeout=10)
            finally:
                self.command("rm", "-f", name, check=False, timeout=15)
        for name in reversed(self.volumes):
            self.command("volume", "rm", name, check=False, timeout=15)


@pytest.fixture
def docker():
    if not shutil.which("docker"):
        pytest.fail("Docker CLI is required for this family")
    client = Docker()
    client.command("info", "--format", "{{.ServerVersion}}", timeout=15)
    client.command("image", "inspect", client.image, "--format", "{{.Id}}", timeout=15)
    try:
        yield client
    finally:
        client.close()
