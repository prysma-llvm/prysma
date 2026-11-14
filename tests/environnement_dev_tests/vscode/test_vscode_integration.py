import json
import os
import shutil
import signal
import subprocess
from pathlib import Path

import pytest

from common.scripts import free_port

pytestmark = [pytest.mark.vscode, pytest.mark.docker]


def test_vscode_debugging_and_failed_build(docker, repo_root, tmp_path):
    code = os.environ.get("PRYSMA_VSCODE_BIN") or shutil.which("code") or shutil.which("code.cmd")
    assert code, "VS Code is required; set PRYSMA_VSCODE_BIN or add code to PATH"
    root = tmp_path / "workspace é with spaces"
    root.mkdir()
    for directory in ("src", "include", "generation", "docker", ".vscode", "tests"):
        shutil.copytree(repo_root / directory, root / directory,
                        ignore=shutil.ignore_patterns("build", "build-*", "node_modules", "__pycache__", ".pytest_cache"))
    for name in ("CMakeLists.txt", "debug.py", "requirements.txt"):
        shutil.copy2(repo_root / name, root / name)
    volume = docker.volume()
    docker.run("chmod", "777", "/tmp/.ccache", options=["-v", f"{volume}:/tmp/.ccache"])
    python_port, lldb_port = free_port(), free_port()
    tasks_path = root / ".vscode/tasks.json"
    tasks = json.loads(tasks_path.read_text())
    for task in tasks["tasks"]:
        for variant in (task, task.get("windows", {})):
            if "command" in variant:
                variant["command"] = variant["command"].replace("prysma-debug", docker.image).replace("prysma-ccache", volume)
                variant["command"] = variant["command"].replace(" run --rm", f" run --rm --label prysma.test={docker.run_id}")
                variant["command"] = variant["command"].replace("-e HOME=/tmp", "-e HOME=/tmp -e PRYSMA_BUILD_JOBS=2")
        if "Debug Server" in task["label"] or "LLDB Server" in task["label"] or task["label"].startswith("Prepare "):
            task.setdefault("options", {}).setdefault("env", {})["PRYSMA_DEBUG_PORT"] = str(python_port if "Python" in task["label"] else lldb_port)
    tasks_path.write_text(json.dumps(tasks, indent=4), encoding="utf-8")
    launch_path = root / ".vscode/launch.json"
    launch_path.write_text(launch_path.read_text().replace("5678", str(python_port)).replace("1234", str(lldb_port)), encoding="utf-8")
    user_dir, extension_dir = tmp_path / "user", tmp_path / "extensions"
    (user_dir / "User").mkdir(parents=True)
    (user_dir / "User/settings.json").write_text(json.dumps({"extensions.autoUpdate": False, "extensions.autoCheckUpdates": False,
                                                          "update.mode": "none", "telemetry.telemetryLevel": "off"}))
    profile = [f"--user-data-dir={user_dir}", f"--extensions-dir={extension_dir}"]
    env = dict(os.environ, PRYSMA_DEBUG_IMAGE=docker.image, PRYSMA_TEST_RUN_ID=docker.run_id,
               PRYSMA_VSCODE_REPORT=str(tmp_path / "vscode-results.json"))
    for key in ("VSCODE_IPC_HOOK_CLI", "VSCODE_PID", "ELECTRON_RUN_AS_NODE"):
        env.pop(key, None)
    version = subprocess.run([code, "--version"], env=env, capture_output=True, text=True, timeout=15)
    assert version.returncode == 0, version.stderr
    print("VS Code:", version.stdout)
    # Versioned extensions in a disposable profile; never install into the developer's profile.
    for extension in ("ms-python.python@2024.22.2", "vadimcn.vscode-lldb@1.11.4", "ms-python.debugpy@2024.14.0"):
        result = subprocess.run([code, *profile, "--install-extension", extension, "--force"],
                                env=env, capture_output=True, text=True, timeout=180)
        assert result.returncode == 0, result.stdout + result.stderr
    command = [code, *profile, "--wait", "--new-window", "--disable-workspace-trust", "--skip-welcome",
               "--skip-release-notes", "--disable-gpu", "--no-sandbox", "--disable-gpu-sandbox",
               f"--extensionDevelopmentPath={repo_root / 'tests/environnement_dev_tests/vscode'}",
               f"--extensionTestsPath={repo_root / 'tests/environnement_dev_tests/vscode/index.js'}", str(root)]
    report_path = tmp_path / "vscode-results.json"
    process = subprocess.Popen(command, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               text=True, start_new_session=os.name != "nt")
    try:
        output, _ = process.communicate(timeout=1200)
        print(output)
    finally:
        if process.poll() is None:
            if os.name == "nt":
                subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"], timeout=20)
            else:
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
            process.wait(timeout=10)
        if report_path.exists():
            print(report_path.read_text())
            if os.environ.get("PRYSMA_TEST_RESULTS_DIR"):
                shutil.copy2(report_path, Path(os.environ["PRYSMA_TEST_RESULTS_DIR"]) / "vscode-scenarios.json")
    assert process.returncode == 0, "VS Code integration host failed"
    assert report_path.exists(), "VS Code exited without running the integration tests"
    report = json.loads(report_path.read_text())
    assert len(report["report"]) == 3 and all(test["status"] == "passed" for test in report["report"])
