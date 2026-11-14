import os
import socket
import subprocess
import uuid

import pytest

from common.dap_client import DAPClient
from common.scripts import debug_script

pytestmark = pytest.mark.linux


def test_scripts_valid_syntax(repo_root):
    for script in (repo_root / "docker" / "environnement_dev" / "linux").glob("*.sh"):
        if script.name != "docker-debug-common.sh":
            assert os.access(script, os.X_OK), f"Script is not executable: {script}"
        result = subprocess.run(["bash", "-n", str(script)], capture_output=True, text=True, timeout=10)
        assert result.returncode == 0, result.stderr


@pytest.mark.docker
def test_cli_exit_code(docker, workspace):
    cli = workspace / "docker" / "environnement_dev" / "linux" / "docker-cli.sh"
    env = dict(os.environ, PRYSMA_DOCKER_RAW="1")
    for code in (0, 42):
        result = subprocess.run(["bash", str(cli), "run", "--rm", docker.image,
                                 "sh", "-c", f"exit {code}"], env=env, capture_output=True, text=True, timeout=30)
        assert result.returncode == code, result.stderr


@pytest.mark.parametrize("name,target", [("python", "missing.py"), ("lldb", "--tests")])
def test_missing_target_fails(workspace, name, target):
    script = workspace / "docker" / "environnement_dev" / "linux" / f"docker-debug-{name}.sh"
    result = subprocess.run(["bash", str(script), target], capture_output=True, text=True, timeout=15)
    assert result.returncode != 0
    assert "ERROR:" in result.stderr
    assert "ready on" not in result.stdout


@pytest.mark.docker
@pytest.mark.parametrize("failure", ["missing-image", "startup-timeout", "missing-daemon"])
def test_startup_errors_are_not_success(docker, workspace, failure):
    script = workspace / "docker/environnement_dev/linux/docker-debug-python.sh"
    env = dict(os.environ, PRYSMA_DEBUG_IMAGE=docker.image, PRYSMA_TEST_RUN_ID=docker.run_id)
    if failure == "missing-image":
        env["PRYSMA_DEBUG_IMAGE"] = f"prysma-test-missing-{uuid.uuid4().hex}"
    elif failure == "startup-timeout":
        env["PRYSMA_DEBUG_START_TIMEOUT"] = "0"
    else:
        env["DOCKER_HOST"] = f"unix://{workspace}/missing-docker.sock"
    result = subprocess.run(["bash", str(script), "fixtures/debug_target.py"], env=env,
                            capture_output=True, text=True, timeout=45)
    assert result.returncode != 0
    assert "ready on" not in result.stdout
    assert result.stderr.strip()
    remaining = docker.command("ps", "-aq", "--filter", f"label=prysma.workspace={workspace}")
    assert not remaining.stdout.strip(), "Failed launch left a debug container behind"


@pytest.mark.docker
def test_python_script_session_and_relaunch(docker, workspace, tmp_path):
    for _ in range(2):
        with debug_script(workspace, docker, tmp_path) as (process, port, _):
            client = DAPClient(port=port)
            try:
                assert client.connect_and_initialize()
                attach = client.send_request("attach", {"pathMappings": [{"localRoot": str(workspace), "remoteRoot": "/workspace"}]})
                assert client.wait_for_event("initialized")
                client.request("configurationDone")
                assert client.wait_for_response(attach)["success"]
                assert client.wait_for_event("terminated")
                assert process.wait(timeout=15) == 0
            finally:
                client.close()
        with socket.socket() as sock:
            sock.settimeout(1)
            assert sock.connect_ex(("127.0.0.1", port)) != 0


@pytest.mark.docker
def test_second_launch_preserves_first_session(docker, workspace, tmp_path):
    with debug_script(workspace, docker, tmp_path) as (process, port, env):
        script = workspace / "docker" / "environnement_dev" / "linux" / "docker-debug-python.sh"
        second = subprocess.run(["bash", str(script), "fixtures/debug_target.py"], env=env,
                                capture_output=True, text=True, timeout=30)
        assert second.returncode != 0 and "ready on" not in second.stdout
        assert process.poll() is None
        client = DAPClient(port=port)
        try:
            assert client.connect_and_initialize(), "First debugger was destroyed by second launch"
        finally:
            client.close()


@pytest.mark.docker
def test_occupied_port_does_not_report_ready(docker, workspace):
    script = workspace / "docker" / "environnement_dev" / "linux" / "docker-debug-python.sh"
    with socket.socket() as sock:
        sock.bind(("0.0.0.0", 0))
        sock.listen()
        env = dict(os.environ, PRYSMA_DEBUG_PORT=str(sock.getsockname()[1]),
                   PRYSMA_DEBUG_IMAGE=docker.image, PRYSMA_TEST_RUN_ID=docker.run_id)
        result = subprocess.run(["bash", str(script), "fixtures/debug_target.py"], env=env,
                                capture_output=True, text=True, timeout=30)
        assert result.returncode != 0
        assert "ready on" not in result.stdout
        assert sock.getsockname()[1] > 0


@pytest.mark.docker
def test_lldb_script_arguments_and_working_directory(docker, workspace, tmp_path):
    (workspace / "build").mkdir()
    (workspace / "source avec é.p").write_text("fixture", encoding="utf-8")
    (workspace / "target.cpp").write_text(
        '#include <cstdio>\n#include <unistd.h>\nint main(int argc, char **argv) {\n'
        'char cwd[4096]; getcwd(cwd, sizeof(cwd));\n'
        'printf("target=%s;cwd=%s\\n", argv[1], cwd);\nreturn argc == 2 ? 0 : 1;\n}\n',
        encoding="utf-8")
    docker.run("clang++", "-g", "-O0", "/workspace/target.cpp", "-o", "/workspace/build/Prysma",
               options=["-v", f"{workspace}:/workspace", "--user", f"{os.getuid()}:{os.getgid()}"])
    with debug_script(workspace, docker, tmp_path, "lldb", str(workspace / "source avec é.p")) as (process, port, _):
        result = docker.run("lldb", "--batch", "-o", "platform select remote-linux",
                            "-o", "target create /workspace/build/Prysma",
                            "-o", f"gdb-remote 127.0.0.1:{port}",
                            "-o", "breakpoint set --file target.cpp --line 5", "-o", "continue",
                            "-o", "frame variable argc", "-o", "continue",
                            options=["--network", "host", "-v", f"{workspace}:/workspace"])
        assert "stop reason = breakpoint" in result.stdout
        assert "argc = 2" in result.stdout
        assert "status = 0" in result.stdout
        assert process.wait(timeout=15) == 0
    assert "target=/workspace/source avec é.p;cwd=/workspace" in result.stdout
