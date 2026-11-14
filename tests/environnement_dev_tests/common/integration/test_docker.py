import os
import sys

import pytest

pytestmark = pytest.mark.docker


def test_image_tools_and_environment(docker):
    result = docker.run("sh", "-ec",
                        "command -v clang++ cmake python3 gdb bear ccache; "
                        "test -x /usr/lib/llvm-18/bin/lldb-server-18; "
                        "python3 -c 'import debugpy; print(debugpy.__version__)'; "
                        "test \"$LLVM_DIR\" = /usr/lib/llvm-18/lib/cmake/llvm; "
                        "test \"$CCACHE_DIR\" = /tmp/.ccache")
    assert result.stdout.strip()


def test_workspace_volume_and_permissions(docker, workspace):
    (workspace / "input.txt").write_text("host é", encoding="utf-8")
    options = ["-v", f"{workspace}:/workspace", "-w", "/workspace"]
    if sys.platform == "linux":
        options += ["--user", f"{os.getuid()}:{os.getgid()}"]
    result = docker.run("sh", "-ec", "cat input.txt; mkdir build; echo container > build/output.txt", options=options)
    assert "host é" in result.stdout
    output = workspace / "build" / "output.txt"
    assert output.read_text().strip() == "container"
    if sys.platform == "linux":
        assert output.stat().st_uid == os.getuid()
        assert output.stat().st_gid == os.getgid()
    output.write_text("modified", encoding="utf-8")
    assert docker.run("cat", "/workspace/build/output.txt", options=options).stdout == "modified"
    output.unlink()
    docker.run("test", "!", "-f", "/workspace/build/output.txt", options=options)


def test_read_only_mount(docker, workspace):
    result = docker.run("sh", "-c", "echo forbidden > /workspace/output",
                        options=["-v", f"{workspace}:/workspace:ro"], check=False)
    assert result.returncode != 0
    assert "Read-only file system" in result.stderr
    assert not (workspace / "output").exists()


def test_unwritable_directory(docker):
    result = docker.run("sh", "-c", "echo forbidden > /root/output",
                        options=["--user", "65534:65534"], check=False)
    assert result.returncode != 0
    assert "Permission denied" in result.stderr


def cache_stats(docker, options):
    output = docker.run("ccache", "--print-stats", options=options).stdout
    return {key: int(value) for key, value in (line.split() for line in output.splitlines())}


def compile_cached(docker, options, value):
    source = f'#include <stdio.h>\nint main() {{ printf("%d", {value}); }}\n'
    return docker.run("sh", "-ec",
                      'printf "%s" "$1" > /tmp/test.cpp; ccache clang++ -c /tmp/test.cpp -o /tmp/test.o; '
                      'clang++ /tmp/test.o -o /tmp/test; /tmp/test',
                      "sh", source, options=options).stdout


def test_ccache_empty_hit_and_changed_source(docker):
    volume = docker.volume()
    options = ["-v", f"{volume}:/tmp/.ccache"]
    docker.run("ccache", "--zero-stats", options=options)
    assert cache_stats(docker, options)["cache_miss"] == 0
    assert compile_cached(docker, options, 42) == "42"
    first = cache_stats(docker, options)
    assert first["cache_miss"] == 1
    # Every call creates a new container, so this cannot be an incremental build.
    assert compile_cached(docker, options, 42) == "42"
    second = cache_stats(docker, options)
    assert second["direct_cache_hit"] + second["preprocessed_cache_hit"] == 1
    assert compile_cached(docker, options, 99) == "99"
    assert cache_stats(docker, options)["cache_miss"] == 2


def test_ccache_unwritable(docker):
    result = docker.run("sh", "-ec",
                        "echo 'int value;' > /tmp/test.cpp; ccache clang++ -c /tmp/test.cpp -o /tmp/test.o",
                        options=["--user", "65534:65534", "-e", "CCACHE_DIR=/root/cache"], check=False)
    assert result.returncode != 0
    assert "Permission denied" in result.stderr


@pytest.mark.parametrize("command, expected", [("exit 0", 0), ("echo deliberate >&2; exit 42", 42), ("kill -TERM $$", 143)])
def test_exit_code_and_logs(docker, command, expected):
    result = docker.run("sh", "-c", command, options=["--init"], check=False)
    assert result.returncode == expected
    if expected == 42:
        assert "deliberate" in result.stderr


def test_exec_logs_and_stopped_container(docker):
    container = docker.run("sh", "-c", "echo dedicated-log; sleep 60", detach=True)
    assert docker.command("exec", container, "printf", "dedicated-exec").stdout == "dedicated-exec"
    assert "dedicated-log" in docker.command("logs", container).stdout
    docker.command("stop", "--time", "1", container)
    assert docker.command("exec", container, "true", check=False).returncode != 0
    assert "dedicated-log" in docker.command("logs", container).stdout
    docker.command("rm", container)
    assert docker.command("logs", container, check=False).returncode != 0
