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
