import json
import os
import shutil
import subprocess

import pytest

from common.lsp_client import LSPClient

pytestmark = [pytest.mark.linux, pytest.mark.docker]


def test_clangd_analysis_with_docker_database(docker, workspace, tmp_path, repo_root):
    shutil.copy2(repo_root / ".clangd", workspace / ".clangd")
    clangd = shutil.which("clangd") or shutil.which("clangd-18")
    assert clangd, "Host clangd is required"
    source = workspace / "main.cpp"
    text = '#include <llvm/ADT/StringRef.h>\nint answer() { return 42; }\nint main() { llvm::StringRef name("prysma"); (void)name; return answer(); }\n'
    source.write_text(text, encoding="utf-8")
    (workspace / "CMakeLists.txt").write_text(
        'cmake_minimum_required(VERSION 3.12)\nproject(Probe)\n'
        'set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\nadd_library(probe OBJECT main.cpp)\n'
        'target_include_directories(probe SYSTEM PRIVATE /usr/lib/llvm-18/include)\n', encoding="utf-8")
    cli = workspace / "docker/environnement_dev/linux/docker-cli.sh"
    # Real CMake generation and wrapper conversion; the wrapper only touches this temporary workspace.
    result = subprocess.run(["bash", str(cli), "run", "--rm", "--user", f"{os.getuid()}:{os.getgid()}",
                             "-v", f"{workspace}:/workspace", "-w", "/workspace", docker.image,
                             "sh", "-ec", "cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++; cmake --build build"],
                            env=dict(os.environ, PRYSMA_DEBUG_IMAGE=docker.image),
                            capture_output=True, text=True, timeout=120)
    assert result.returncode == 0, result.stdout + result.stderr
    database = workspace / "build/compile_commands.json"
    commands = json.loads(database.read_text())
    assert commands[0]["file"] == str(source)
    assert all("/workspace" not in argument for argument in commands[0]["arguments"])
    assert (workspace / ".clangd-includes/llvm/ADT/StringRef.h").is_file()
    with (tmp_path / "clangd.log").open("w") as log:
        process = subprocess.Popen([clangd, f"--compile-commands-dir={workspace / 'build'}", "--background-index=false"],
                                   stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=log)
        client = LSPClient(process)
        uri = source.as_uri()
        try:
            client.request("initialize", {"processId": os.getpid(), "rootUri": workspace.as_uri(), "capabilities": {}})
            client.send("initialized", {})
            client.send("textDocument/didOpen", {"textDocument": {"uri": uri, "languageId": "cpp", "version": 1, "text": text}})
            diagnostics = client.wait(lambda message: message.get("method") == "textDocument/publishDiagnostics"
                                      and message["params"]["uri"] == uri)
            assert not diagnostics["params"]["diagnostics"], diagnostics
            position = {"textDocument": {"uri": uri}, "position": {"line": 2, "character": text.splitlines()[2].index("answer") + 2}}
            hover = client.request("textDocument/hover", position)
            assert "int" in json.dumps(hover), hover
            definition = client.request("textDocument/definition", position)
            assert definition and definition[0]["uri"] == uri
            assert definition[0]["range"]["start"]["line"] == 1
            completions = client.request("textDocument/completion", position)
            assert "answer" in json.dumps(completions)
            broken = text.replace("return answer();", "return unknown_symbol;")
            client.send("textDocument/didChange", {"textDocument": {"uri": uri, "version": 2}, "contentChanges": [{"text": broken}]})
            errors = client.wait(lambda message: message.get("method") == "textDocument/publishDiagnostics"
                                 and message["params"].get("version") == 2)
            assert any("unknown_symbol" in error["message"] for error in errors["params"]["diagnostics"])
            client.send("textDocument/didChange", {"textDocument": {"uri": uri, "version": 3}, "contentChanges": [{"text": text}]})
            fixed = client.wait(lambda message: message.get("method") == "textDocument/publishDiagnostics"
                                and message["params"].get("version") == 3)
            assert not fixed["params"]["diagnostics"]
            client.request("shutdown", None)
            client.send("exit", None)
            assert process.wait(timeout=10) == 0
        finally:
            if process.poll() is None:
                process.kill()
            process.wait(timeout=10)
            client.reader.join(timeout=5)
            print((tmp_path / "clangd.log").read_text())
