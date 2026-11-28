import json
import os
import subprocess

import pytest

pytestmark = pytest.mark.windows


def powershell(script, *args, env=None):
    result = subprocess.run(["powershell.exe", "-NoProfile", "-NonInteractive", "-ExecutionPolicy", "Bypass",
                             "-File", str(script), *args], env=env, capture_output=True, text=True,
                            encoding="utf-8", errors="replace", timeout=90)
    print(result.stdout, result.stderr)
    return result


def test_powershell_scripts_parse(repo_root, tmp_path):
    parser = tmp_path / "parse.ps1"
    parser.write_text(
        '$errors = $null; $tokens = $null\n'
        'Get-ChildItem -LiteralPath $args[0] -Filter *.ps1 | ForEach-Object {\n'
        ' [System.Management.Automation.Language.Parser]::ParseFile($_.FullName, [ref]$tokens, [ref]$errors) | Out-Null\n'
        ' if ($errors.Count) { Write-Error ($errors | Out-String); exit 1 }\n}\n', encoding="utf-8")
    result = powershell(parser, str(repo_root / "docker/environnement_dev/windows"))
    assert result.returncode == 0, result.stderr


def test_missing_target_fails(workspace):
    script = workspace / "docker/environnement_dev/windows/docker-debug-python.ps1"
    result = powershell(script, "--", "missing.py")
    assert result.returncode != 0
    assert "Target does not exist" in result.stderr
    assert "ready on" not in result.stdout


@pytest.mark.parametrize("build_exit", [0, 42])
def test_build_without_host_python(workspace, repo_root, tmp_path, build_exit):
    # Exercise the actual VS Code command in Windows PowerShell, even on runners
    # that cannot boot WSL. The full VS Code suite exercises the real engine.
    tasks = json.loads((repo_root / ".vscode/tasks.json").read_text(encoding="utf-8"))["tasks"]
    command = next(task["windows"]["command"] for task in tasks if task["label"] == "Build")
    header = workspace / ".clangd-includes/llvm/Support/TargetSelect.h"
    header.parent.mkdir(parents=True)
    header.touch()
    script = tmp_path / "build-task.ps1"
    script.write_text(r'''
function python { throw "Build invoked host Python" }
function docker.exe {
    $global:LASTEXITCODE = 0
    if ($args[0] -in @("volume", "version")) { return }
    if ($args -contains "debug.py") {
        Write-Output "compiler executed"
        $global:LASTEXITCODE = [int]$env:TEST_BUILD_EXIT
        return
    }
    if ($args -contains "/workspace/docker/environnement_dev/compile_commands.py" -and
        $args -contains "python3" -and $args[-1] -ceq $env:TEST_HOST_ROOT) {
        Write-Output "container indexer executed"
        return
    }
    throw "Unexpected Docker invocation: $args"
}
Set-Alias docker docker.exe
Set-Location -LiteralPath $env:TEST_HOST_ROOT
''' + command.replace("${workspaceFolder}", str(workspace)) + "\nexit $LASTEXITCODE\n",
                      encoding="utf-8-sig")
    env = dict(os.environ, PRYSMA_DOCKER_RAW="0", TEST_BUILD_EXIT=str(build_exit),
               TEST_HOST_ROOT=workspace.as_posix())
    result = powershell(script, env=env)
    assert result.returncode == build_exit, result.stderr
    assert "compiler executed" in result.stdout
    assert ("container indexer executed" in result.stdout) == (build_exit == 0)


@pytest.mark.docker
def test_powershell_paths_and_arguments(docker, workspace):
    script = workspace / "docker/environnement_dev/windows/docker-cli.ps1"
    (workspace / "entrée avec espaces.txt").write_text("contenu é", encoding="utf-8")
    env = dict(os.environ, PRYSMA_DOCKER_RAW="1")
    result = powershell(script, "--", "run", "--rm", "-v", f"{workspace}:/workspace", docker.image,
                        "cat", "/workspace/entrée avec espaces.txt", env=env)
    assert result.returncode == 0, result.stderr
    assert result.stdout.strip() == "contenu é"
    result = powershell(script, "--", "run", "--rm", docker.image, "sh", "-c", "exit 42", env=env)
    assert result.returncode == 42
