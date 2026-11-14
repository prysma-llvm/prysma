# Shared Docker debugger lifecycle for native Windows.
$ErrorActionPreference = "Stop"

function Invoke-DebugDocker {
    & docker.exe @args
    if ($LASTEXITCODE -ne 0) {
        throw "Docker failed with exit code $LASTEXITCODE"
    }
}

function Start-PrysmaDebug {
    param([string]$Kind, [string]$Target)
    $root = (Resolve-Path (Join-Path $PSScriptRoot "../../..")).Path
    $forwardRoot = $root.Replace("\", "/")
    $hash = [System.Security.Cryptography.SHA256]::Create()
    try {
        $digest = [BitConverter]::ToString($hash.ComputeHash([Text.Encoding]::UTF8.GetBytes($root))).Replace("-", "").ToLower()
    } finally {
        $hash.Dispose()
    }
    $name = "prysma-$Kind-" + $digest.Substring(0, 16)
    if ($Target -eq "--stop") {
        $ErrorActionPreference = "Continue"
        $id = & docker.exe inspect --format '{{.Id}}' $name 2>$null
        $ErrorActionPreference = "Stop"
        if ($LASTEXITCODE -ne 0) { return }
        $labels = Invoke-DebugDocker inspect --format '{{json .Config.Labels}}' $id | ConvertFrom-Json
        $owner = $labels.'prysma.workspace'
        if ($owner -ne $root) { throw "Refusing to stop a container owned by another workspace" }
        Invoke-DebugDocker stop --time 2 $id | Out-Null
        return
    }

    $image = if ($env:PRYSMA_DEBUG_IMAGE) { $env:PRYSMA_DEBUG_IMAGE } else { "prysma-debug" }
    $port = if ($env:PRYSMA_DEBUG_PORT) { [int]$env:PRYSMA_DEBUG_PORT } elseif ($Kind -eq "python") { 5678 } else { 1234 }
    $deadline = if ($env:PRYSMA_DEBUG_START_TIMEOUT) { [double]$env:PRYSMA_DEBUG_START_TIMEOUT } else { 15 }
    $options = @("-p", "127.0.0.1:${port}:$port", "-v", "${forwardRoot}:/workspace", "-w", "/workspace", "-e", "HOME=/tmp")
    if ($Kind -eq "lldb" -and $Target -in @("--tests", "tests")) {
        $binary = "tests/unit_tests/build/PrysmaTests"
        $program = @("/workspace/$binary")
        $options += @("-w", "/workspace/tests/unit_tests")
    } else {
        $extension = if ($Kind -eq "python") { ".py" } else { ".p" }
        if (-not $Target -or -not $Target.EndsWith($extension)) {
            $Target = if ($Kind -eq "python") { "debug.py" } else { "tests/prysma_code_tests/project_test_multi_thread_edge_case_file/test_depth/main.p" }
        }
        $Target = $Target.Replace("\", "/") -replace '^/?workspace/', ''
        $local = if ([IO.Path]::IsPathRooted($Target)) { [IO.Path]::GetFullPath($Target) } else { [IO.Path]::GetFullPath((Join-Path $root $Target)) }
        if (-not $local.StartsWith($root + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) {
            throw "Target must be inside the workspace"
        }
        if (-not (Test-Path -LiteralPath $local -PathType Leaf)) { throw "Target does not exist: $local" }
        $remote = "/workspace/" + $local.Substring($root.Length + 1).Replace("\", "/")
        $binary = "build/Prysma"
        $program = @("/workspace/$binary", $remote)
        if ($Kind -eq "python") { $program = @($remote) }
    }
    if ($Kind -eq "lldb") {
        if (-not (Test-Path -LiteralPath (Join-Path $root $binary) -PathType Leaf)) { throw "Target binary does not exist: $binary" }
        $options += @("--cap-add=SYS_PTRACE", "--security-opt", "seccomp=unconfined", "-e", "ASAN_OPTIONS=detect_leaks=0")
        $command = @("/usr/lib/llvm-18/bin/lldb-server-18", "gdbserver", "0.0.0.0:$port", "--") + $program
        $vscode = Join-Path $root ".vscode"
        New-Item -ItemType Directory -Force $vscode | Out-Null
        [IO.File]::WriteAllText((Join-Path $vscode "lldb_source_map.txt"),
            "settings set target.source-map /workspace `"$forwardRoot`"`n",
            (New-Object Text.UTF8Encoding $false))
        $ready = "lldb-server ready"
    } else {
        $options += @("-e", "PYTHONDONTWRITEBYTECODE=1")
        $command = @("python3", "-m", "debugpy", "--listen", "0.0.0.0:$port", "--wait-for-client") + $program
        $ready = "debugpy server ready"
    }
    $labels = @("--label", "prysma.workspace=$root")
    if ($env:PRYSMA_TEST_RUN_ID) { $labels += @("--label", "prysma.test=$env:PRYSMA_TEST_RUN_ID") }
    $id = $null
    try {
        $id = Invoke-DebugDocker create --pull=never --name $name @labels @options $image @command
        $id = $id.Trim()
        Invoke-DebugDocker start $id | Out-Null
        $probe = @'
import pathlib, sys, time
port, seconds = int(sys.argv[1]), float(sys.argv[2])
deadline = time.monotonic() + seconds
while time.monotonic() < deadline:
    for table in ('/proc/net/tcp', '/proc/net/tcp6'):
        for row in pathlib.Path(table).read_text().splitlines()[1:]:
            fields = row.split()
            if int(fields[1].split(':')[1], 16) == port and fields[3] == '0A':
                sys.exit(0)
    time.sleep(0.1)
sys.exit('Debug server readiness deadline exceeded')
'@
        Invoke-DebugDocker exec $id python3 -c $probe $port $deadline
        Write-Output "$ready on $port"
        Invoke-DebugDocker logs -f $id
        $status = Invoke-DebugDocker wait $id
        if ([int]$status -ne 0) { throw "Debug container exited with code $status" }
    } catch {
        if ($id) { & docker.exe logs $id }
        throw
    } finally {
        if ($id) { & docker.exe rm -f $id | Out-Null }
    }
}
