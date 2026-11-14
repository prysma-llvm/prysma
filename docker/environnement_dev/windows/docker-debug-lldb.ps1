# Docker LLDB server for Prysma and its unit tests.
. (Join-Path $PSScriptRoot "docker-debug-common.ps1")
$target = @($args | Where-Object { $_ -ne "--" } | Select-Object -First 1)
try {
    Start-PrysmaDebug -Kind "lldb" -Target ($target | Select-Object -First 1)
} catch {
    [Console]::Error.WriteLine("ERROR: $_")
    exit 1
}
