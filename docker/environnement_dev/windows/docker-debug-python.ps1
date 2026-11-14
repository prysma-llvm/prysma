# Docker debugpy server for the current workspace.
. (Join-Path $PSScriptRoot "docker-debug-common.ps1")
$target = @($args | Where-Object { $_ -ne "--" } | Select-Object -First 1)
try {
    Start-PrysmaDebug -Kind "python" -Target ($target | Select-Object -First 1)
} catch {
    [Console]::Error.WriteLine("ERROR: $_")
    exit 1
}
