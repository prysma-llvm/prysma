# docker/windows/docker-debug-python.ps1 - Python debugpy server for Windows 10/11
$ErrorActionPreference = "Continue"

if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    [Console]::Error.WriteLine("Erreur: Docker est introuvable sur le système hôte. Veuillez installer et démarrer Docker Desktop.")
    exit 127
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RootDir = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$ForwardRootDir = $RootDir.Replace("\", "/")

$Target = ""
foreach ($a in $args) {
    if ($a -ne '--' -and -not [string]::IsNullOrWhiteSpace($a)) {
        $Target = $a
        break
    }
}
if (-not $Target -or (-not $Target.EndsWith(".py"))) {
    $Target = "debug.py"
}
$Target = $Target.Replace("\", "/")
if ($Target.StartsWith("/workspace/")) {
    $Target = $Target.Substring(11)
} elseif ($Target.StartsWith("workspace/")) {
    $Target = $Target.Substring(10)
}

& docker.exe rm -f prysma-python-debug 2>$null | Out-Null
Start-Sleep -Milliseconds 200

& docker.exe run -d --rm --name prysma-python-debug `
    -p 5678:5678 -e HOME=/tmp `
    -v "${ForwardRootDir}:/workspace" -w /workspace `
    prysma-debug python3 -m debugpy --listen 0.0.0.0:5678 --wait-for-client "$Target"

# Wait for debugpy port 5678 to be listening
$timeout = 60
$connected = $false
while ($timeout -gt 0) {
    try {
        $tcp = New-Object System.Net.Sockets.TcpClient
        $tcp.Connect("127.0.0.1", 5678)
        $tcp.Close()
        $tcp.Dispose()
        $connected = $true
        break
    } catch {
        if ($tcp) { $tcp.Dispose() }
        Start-Sleep -Milliseconds 100
        $timeout--
    }
}

if (-not $connected) {
    [Console]::Error.WriteLine("Avertissement: Le serveur debugpy n'a pas répondu sur le port 5678 dans les délais.")
}
