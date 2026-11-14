from contextlib import contextmanager
import os
import signal
import socket
import subprocess
import time


def free_port():
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


@contextmanager
def debug_script(workspace, docker, tmp_path, name="python", target="fixtures/debug_target.py", port=None):
    port = port or free_port()
    env = dict(os.environ, PRYSMA_DEBUG_PORT=str(port), PRYSMA_DEBUG_IMAGE=docker.image,
               PRYSMA_TEST_RUN_ID=docker.run_id)
    command = ["bash", str(workspace / "docker" / "environnement_dev" / "linux" / f"docker-debug-{name}.sh"), target]
    log_path = tmp_path / f"{name}-{port}.log"
    with log_path.open("w", encoding="utf-8") as log:
        process = subprocess.Popen(command, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
        try:
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                output = log_path.read_text(encoding="utf-8")
                assert process.poll() is None, output
                if f"ready on {port}" in output:
                    break
                time.sleep(0.1)
            else:
                raise AssertionError(f"Debug script readiness timed out: {output}")
            yield process, port, env
        finally:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGTERM)
                try:
                    process.wait(timeout=15)
                except subprocess.TimeoutExpired:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.wait(timeout=5)
            print(log_path.read_text(encoding="utf-8"))
