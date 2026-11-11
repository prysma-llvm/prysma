# docker/windows/docker-debug-lldb.ps1 - LLDB Server starter for Windows 10/11
$ErrorActionPreference = "Continue"

# Check Docker availability
if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    [Console]::Error.WriteLine("Erreur: Docker CLI est introuvable sur le système hôte. Veuillez installer Docker Desktop.")
    exit 127
}

& docker.exe version 2>$null | Out-Null
if ($LASTEXITCODE -ne 0) {
    [Console]::Error.WriteLine("")
    [Console]::Error.WriteLine("================================================================================")
    [Console]::Error.WriteLine("ERREUR : Docker Desktop n'est pas demarre.")
    [Console]::Error.WriteLine("1. Ouvrez l'application 'Docker Desktop' depuis votre menu Demarrer Windows.")
    [Console]::Error.WriteLine("2. Attendez que le statut affiche 'Engine running' (icone verte en bas).")
    [Console]::Error.WriteLine("3. Relancez ensuite votre debogage.")
    [Console]::Error.WriteLine("================================================================================")
    [Console]::Error.WriteLine("")
    exit 1
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RootDir = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$ForwardRootDir = $RootDir.Replace("\", "/")

# Write forward-slash source-map script for LLDB on Windows
$vscodeDir = Join-Path $RootDir ".vscode"
if (-not (Test-Path $vscodeDir)) {
    New-Item -ItemType Directory -Path $vscodeDir -Force | Out-Null
}
$lldbInitPath = Join-Path $vscodeDir "lldb_source_map.txt"
@(
    "settings set target.source-map /workspace `"$ForwardRootDir`"",
    "process handle -s false -p true -n true SIGSTOP"
) | Set-Content -Path $lldbInitPath -Encoding ASCII

# Determine target Prysma file to debug
$target = ""
foreach ($a in $args) {
    if ($a -ne '--' -and -not [string]::IsNullOrWhiteSpace($a)) {
        $target = $a
        break
    }
}
if (-not $target -or (-not $target.EndsWith(".p"))) {
    $target = "tests/prysma_code_tests/project_test_multi_thread_edge_case_file/test_depth/main.p"
}
# Normalize Windows backslashes to forward slashes for Linux container path
$target = $target.Replace("\", "/")
if ($target.StartsWith("/workspace/")) {
    $target = $target.Substring(11)
} elseif ($target.StartsWith("workspace/")) {
    $target = $target.Substring(10)
}

# Clean up any existing container and release port 1234
& docker.exe rm -f prysma-lldb-debug prysma-lldb-tests-debug 2>$null | Out-Null
Start-Sleep -Milliseconds 200

# Signal VS Code problem matcher that LLDB server is listening
[Console]::Out.WriteLine("lldb-server ready on 1234")

# Launch LLDB server in container
& docker.exe run --rm --name prysma-lldb-debug -p 1234:1234 --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -v "${ForwardRootDir}:/workspace" -w /workspace -e ASAN_OPTIONS=detect_leaks=0 prysma-debug /usr/lib/llvm-18/bin/lldb-server-18 gdbserver 0.0.0.0:1234 -- /workspace/build/Prysma "/workspace/$target"
