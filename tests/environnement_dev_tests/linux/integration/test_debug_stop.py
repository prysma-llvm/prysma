import hashlib
import os
import shlex
import subprocess

import pytest

pytestmark = [pytest.mark.linux, pytest.mark.docker]


@pytest.fixture(params=["python", "lldb"])
def stop_session(request, docker, workspace):
    digest = hashlib.sha256(str(workspace).encode()).hexdigest()[:16]
    name = f"prysma-{request.param}-{digest}"
    script = workspace / f"docker/environnement_dev/linux/docker-debug-{request.param}.sh"

    def create(owner=workspace):
        docker.containers.append(name)
        return docker.command("run", "-d", "--name", name,
                              "--label", f"prysma.test={docker.run_id}",
                              "--label", f"prysma.workspace={owner}",
                              docker.image, "sleep", "60").stdout.strip()

    def stop(**environment):
        return subprocess.run(["bash", str(script), "--stop"],
                              env=dict(os.environ, **environment),
                              capture_output=True, text=True, timeout=45)

    return create, stop


def test_stop_is_repeatable(stop_session, docker):
    create, stop = stop_session
    absent = stop()
    assert absent.returncode == 0, absent.stderr
    container = create()
    for _ in range(2):
        result = stop()
        assert result.returncode == 0, result.stderr
    assert docker.command("inspect", "--format", "{{.State.Running}}", container).stdout.strip() == "false"


def test_stop_preserves_another_workspaces_container(stop_session, docker):
    create, stop = stop_session
    container = create(owner="another workspace")
    result = stop()
    assert result.returncode != 0
    assert "owned by another workspace" in result.stderr
    assert docker.command("inspect", "--format", "{{.State.Running}}", container).stdout.strip() == "true"


def test_stop_reports_unavailable_daemon(stop_session, workspace):
    _, stop = stop_session
    result = stop(DOCKER_HOST=f"unix://{workspace}/missing-docker.sock")
    assert result.returncode != 0
    assert result.stderr.strip()


@pytest.mark.parametrize("failure", ["removed", "stop-error"])
def test_stop_distinguishes_removal_from_docker_error(stop_session, docker, workspace, failure):
    create, stop = stop_session
    container = create()
    cli = workspace / "docker/environnement_dev/linux/docker-cli.sh"
    real_cli = cli.with_name("docker-cli-real.sh")
    cli.rename(real_cli)
    # Force real Docker removal after lookup/ownership verification, just before stop.
    # All Docker operations still run against the daemon; only their order is controlled.
    hook = ('"$real_cli" rm -f "$PRYSMA_STOP_TEST_CONTAINER" >/dev/null' if failure == "removed"
            else f'export DOCKER_HOST={shlex.quote(f"unix://{workspace}/missing-docker.sock")}')
    cli.write_text(
        '#!/bin/bash\nset -eu\n'
        f'real_cli={shlex.quote(str(real_cli))}\n'
        'if [ "$1" = stop ]; then\n'
        f'    {hook}\n'
        'fi\n'
        'exec "$real_cli" "$@"\n', encoding="utf-8")
    cli.chmod(0o755)
    result = stop(PRYSMA_STOP_TEST_CONTAINER=container)
    if failure == "removed":
        assert result.returncode == 0, result.stderr
        assert not docker.command("ps", "-aq", "--filter", f"id={container}").stdout.strip()
    else:
        assert result.returncode != 0
        assert result.stderr.strip()
        assert docker.command("inspect", "--format", "{{.State.Running}}", container).stdout.strip() == "true"
