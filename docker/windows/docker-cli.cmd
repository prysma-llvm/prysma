@echo off
rem docker/windows/docker-cli.cmd - Wrapper for Windows CMD and external tools
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0docker-cli.ps1" -- %*
