import time

import pytest

from common.dap_client import DAPClient

pytestmark = [pytest.mark.docker, pytest.mark.dap]


def line_number(source, text):
    lines = [index for index, line in enumerate(source.read_text(encoding="utf-8").splitlines(), 1)
             if line.strip() == text]
    assert len(lines) == 1, text
    return lines[0]


@pytest.fixture
def debug_session(docker, workspace):
    clients = []

    def start(delay=0):
        container = docker.run("sh", "-c", 'sleep "$1"; exec python3 -m debugpy --listen 0.0.0.0:5678 '
                               '--wait-for-client /workspace/fixtures/debug_target.py', "sh", str(delay),
                               options=["-p", "127.0.0.1::5678", "-v", f"{workspace}:/workspace",
                                        "-e", "PYTHONDONTWRITEBYTECODE=1"], detach=True)
        port = int(docker.command("port", container, "5678/tcp").stdout.strip().rsplit(":", 1)[1])
        client = DAPClient(port=port)
        clients.append(client)
        assert client.connect_and_initialize(timeout=20), "No successful DAP initialize response"
        return client, container

    yield start
    for client in clients:
        client.close()


def attach(client, workspace, instruction="result = compute_double(int_var)"):
    source = workspace / "fixtures" / "debug_target.py"
    request = client.send_request("attach", {"pathMappings": [{"localRoot": str(workspace), "remoteRoot": "/workspace"}],
                                             "justMyCode": False})
    assert client.wait_for_event("initialized")
    line = line_number(source, instruction)
    breakpoints = client.request("setBreakpoints", {"source": {"path": str(source)}, "breakpoints": [{"line": line}]})
    assert breakpoints["breakpoints"][0]["verified"]
    client.request("configurationDone")
    response = client.wait_for_response(request)
    assert response and response["success"]
    stopped = client.wait_for_event("stopped")
    assert stopped and stopped["body"]["reason"] == "breakpoint"
    thread = stopped["body"]["threadId"]
    frame = client.request("stackTrace", {"threadId": thread})["stackFrames"][0]
    assert frame["line"] == line
    assert frame["source"]["path"] == str(source)
    return thread, frame, source


def evaluate(client, frame, expression):
    return client.request("evaluate", {"frameId": frame["id"], "expression": expression, "context": "watch"})["result"]


def test_debugpy_breakpoints_variables_and_steps(debug_session, workspace):
    client, _ = debug_session()
    thread, frame, source = attach(client, workspace)
    scopes = client.request("scopes", {"frameId": frame["id"]})["scopes"]
    variables = client.request("variables", {"variablesReference": scopes[0]["variablesReference"]})["variables"]
    values = {variable["name"]: variable for variable in variables}
    assert values["int_var"]["value"] == "42"
    assert values["bool_var"]["value"] == "True"
    assert float(values["float_var"]["value"]) == pytest.approx(3.14159)
    assert evaluate(client, frame, "str_var == 'prysma_compiler' and unicode_var == 'éàçü_テスト' and empty_str == ''") == "True"
    assert evaluate(client, frame, "list_var[2] + int_var") == "72"
    items = client.request("variables", {"variablesReference": values["list_var"]["variablesReference"]})["variables"]
    assert any(item["value"] == "30" for item in items)
    unavailable = client.wait_for_response(client.send_request("evaluate", {"frameId": frame["id"], "expression": "not_in_scope", "context": "watch"}))
    assert unavailable and not unavailable["success"]
    client.request("stepIn", {"threadId": thread})
    assert client.wait_for_event("stopped")
    frames = client.request("stackTrace", {"threadId": thread})["stackFrames"]
    assert frames[0]["name"] == "compute_double"
    assert evaluate(client, frames[0], "value") == "42"
    assert evaluate(client, frames[1], "int_var") == "42"
    client.request("next", {"threadId": thread})
    assert client.wait_for_event("stopped")
    frame = client.request("stackTrace", {"threadId": thread})["stackFrames"][0]
    assert evaluate(client, frame, "doubled") == "84"
    client.request("stepOut", {"threadId": thread})
    assert client.wait_for_event("stopped")
    # Move the breakpoint while paused, then verify its new location is reached.
    line = line_number(source, 'print(f"Calculated: {result}")')
    result = client.request("setBreakpoints", {"source": {"path": str(source)}, "breakpoints": [{"line": line}]})
    assert result["breakpoints"][0]["verified"]
    client.request("continue", {"threadId": thread})
    assert client.wait_for_event("stopped")
    frame = client.request("stackTrace", {"threadId": thread})["stackFrames"][0]
    assert frame["line"] == line and evaluate(client, frame, "result") == "84"
    client.request("setBreakpoints", {"source": {"path": str(source)}, "breakpoints": []})
    client.request("continue", {"threadId": thread})
    assert client.wait_for_event("terminated")


def test_debugpy_delayed_start(debug_session, workspace):
    started = time.monotonic()
    client, _ = debug_session(delay=2)
    assert time.monotonic() - started >= 2
    thread, _, _ = attach(client, workspace)
    client.request("continue", {"threadId": thread})
    assert client.wait_for_event("terminated")


def test_debugpy_disconnect_and_restart(debug_session, workspace):
    for _ in range(2):
        client, _ = debug_session()
        attach(client, workspace)
        client.request("disconnect", {"terminateDebuggee": True})
        client.close()


def test_debugpy_server_disappears(debug_session, workspace, docker):
    client, container = debug_session()
    attach(client, workspace)
    docker.command("kill", container)
    with pytest.raises((ConnectionError, OSError)):
        client.request("threads")


def test_dap_rejects_unrelated_server(docker):
    container = docker.run("python3", "-m", "http.server", "5678",
                           options=["-p", "127.0.0.1::5678"], detach=True)
    port = int(docker.command("port", container, "5678/tcp").stdout.strip().rsplit(":", 1)[1])
    client = DAPClient(port=port)
    started = time.monotonic()
    try:
        assert client.connect_and_initialize(timeout=2) is None
        assert time.monotonic() - started < 4
    finally:
        client.close()


def test_lldb_remote_breakpoint_and_variables(docker, workspace):
    source = workspace / "target.cpp"
    source.write_text('''#include <string>
struct Value { int count; };
int twice(int value) {
    int doubled = value * 2;
    return doubled;
}
int main() {
    int number = 42;
    bool flag = true;
    double fraction = 3.5;
    std::string text = "prysma";
    Value object{7};
    Value *pointer = &object;
    Value *empty = nullptr;
    int result = twice(number);
    return result == 84 ? 0 : 1;
}
''', encoding="utf-8")
    # The server and LLDB are real processes in one isolated container.
    script = '''set -eu
clang++ -g -O0 -fstandalone-debug /workspace/target.cpp -o /tmp/target
lldb-server-18 gdbserver 127.0.0.1:1234 -- /tmp/target > /tmp/server.log 2>&1 &
server=$!
trap 'kill "$server" 2>/dev/null || true; cat /tmp/server.log' EXIT
python3 -c 'import pathlib,time
for attempt in range(100):
    if any(row.split()[1].endswith(":04D2") and row.split()[3] == "0A" for row in pathlib.Path("/proc/net/tcp").read_text().splitlines()[1:]): break
    time.sleep(.1)
else: raise SystemExit("LLDB server did not listen")'
lldb --batch -o 'target create /tmp/target' -o 'gdb-remote 127.0.0.1:1234' \\
    -o 'breakpoint set --file target.cpp --line 15' -o continue \\
    -o 'frame variable number flag fraction text object pointer empty' \\
    -o 'expression number + object.count' -o 'thread step-in' \\
    -o 'thread step-over' -o 'frame variable doubled' -o 'thread backtrace' \\
    -o 'frame select 1' -o 'frame variable number' -o 'frame select 0' \\
    -o 'thread step-out' -o continue
'''
    result = docker.run("bash", "-c", script,
                        options=["--cap-add=SYS_PTRACE", "--security-opt", "seccomp=unconfined",
                                 "-v", f"{workspace}:/workspace"])
    output = result.stdout
    for expected in ("stop reason = breakpoint", "number = 42", "flag = true", "fraction = 3.5",
                     '"prysma"', "count = 7", "empty = nullptr", "doubled = 84", "status = 0"):
        assert expected in output, output
