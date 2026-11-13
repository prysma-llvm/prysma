# docker/environnement_dev/windows/docker-stop-python.ps1 - Stop Python debugpy container for Windows 10/11
$ErrorActionPreference = "SilentlyContinue"
& docker.exe rm -f prysma-python-debug 2>$null | Out-Null
exit 0
