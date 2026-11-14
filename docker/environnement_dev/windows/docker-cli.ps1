# docker/environnement_dev/windows/docker-cli.ps1 - Transparent Docker runner and Clangd indexer for Windows 10/11
$ErrorActionPreference = "Continue"

# Lifecycle commands and isolated tests must not populate personal caches.
if ($env:PRYSMA_DOCKER_RAW -eq "1") {
    $rawArgs = @($args | Select-Object -Skip ([int]($args.Count -gt 0 -and $args[0] -eq '--')))
    & docker.exe @rawArgs
    exit $LASTEXITCODE
}

# Check Docker CLI availability
if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    [Console]::Error.WriteLine("Erreur: Docker CLI est introuvable sur le système hôte. Veuillez installer Docker Desktop.")
    exit 127
}

function Ensure-DockerDaemon {
    & docker.exe version 2>$null | Out-Null
    if ($LASTEXITCODE -eq 0) {
        return $true
    }

    $dockerDesktopPath = Join-Path $env:ProgramFiles "Docker\Docker\Docker Desktop.exe"
    if (Test-Path $dockerDesktopPath) {
        [Console]::Out.WriteLine("[Docker] Docker Desktop n'est pas actif. Lancement automatique en cours...")
        Start-Process -FilePath $dockerDesktopPath
        $timeout = 60
        while ($timeout -gt 0) {
            Start-Sleep -Seconds 2
            & docker.exe version 2>$null | Out-Null
            if ($LASTEXITCODE -eq 0) {
                [Console]::Out.WriteLine("[Docker] Docker Desktop est connecte et pret !")
                return $true
            }
            $timeout -= 2
            Write-Host -NoNewline "."
        }
        Write-Host ""
    }

    [Console]::Error.WriteLine("")
    [Console]::Error.WriteLine("================================================================================")
    [Console]::Error.WriteLine("ERREUR : Docker Desktop n'est pas demarre.")
    [Console]::Error.WriteLine("1. Ouvrez l'application 'Docker Desktop' depuis votre menu Demarrer Windows.")
    [Console]::Error.WriteLine("2. Attendez que le statut affiche 'Engine running' (icone verte en bas).")
    [Console]::Error.WriteLine("3. Relancez ensuite votre commande.")
    [Console]::Error.WriteLine("================================================================================")
    [Console]::Error.WriteLine("")
    exit 1
}

Ensure-DockerDaemon | Out-Null

# Capture all raw arguments without PowerShell parameter binding
$rawArgs = @($args)
if ($rawArgs.Count -gt 0 -and $rawArgs[0] -eq '--') {
    if ($rawArgs.Count -gt 1) {
        $rawArgs = $rawArgs[1..($rawArgs.Count - 1)]
    } else {
        $rawArgs = @()
    }
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RootDir = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $ScriptDir))
$image = if ($env:PRYSMA_DEBUG_IMAGE) { $env:PRYSMA_DEBUG_IMAGE } else { "prysma-debug" }

# Ensure persistent ccache only for commands that actually mount it.
if ($rawArgs -contains "prysma-ccache:/tmp/.ccache") {
    & docker.exe volume inspect prysma-ccache 2>$null | Out-Null
    if ($LASTEXITCODE -ne 0) {
        & docker.exe volume create prysma-ccache 2>$null | Out-Null
        & docker.exe run --rm -v prysma-ccache:/tmp/.ccache $image chmod 777 /tmp/.ccache 2>$null | Out-Null
    }
}

# Ensure .clangd-includes is populated for host Clangd indexing
$ClangdIncludesDir = Join-Path $RootDir ".clangd-includes"
if ($rawArgs.Count -gt 0 -and $rawArgs[0] -eq "run" -and -not (Test-Path (Join-Path $ClangdIncludesDir "llvm\Support\TargetSelect.h"))) {
    try {
        $tempContainer = (& docker.exe create $image 2>$null)
        if ($tempContainer -and $LASTEXITCODE -eq 0) {
            $tempContainer = $tempContainer.Trim()
            New-Item -ItemType Directory -Force -Path $ClangdIncludesDir | Out-Null
            $llvmDir = Join-Path $ClangdIncludesDir "llvm"
            $llvmCDir = Join-Path $ClangdIncludesDir "llvm-c"
            New-Item -ItemType Directory -Force -Path $llvmDir | Out-Null
            New-Item -ItemType Directory -Force -Path $llvmCDir | Out-Null
            & docker.exe cp "${tempContainer}:/usr/include/llvm-18/llvm/." "$llvmDir" 2>$null | Out-Null
            & docker.exe cp "${tempContainer}:/usr/include/llvm-c-18/llvm-c/." "$llvmCDir" 2>$null | Out-Null
            & docker.exe rm -f $tempContainer 2>$null | Out-Null
            $llvm18Dir = Join-Path $ClangdIncludesDir "llvm-18"
            if (-not (Test-Path $llvm18Dir)) {
                New-Item -ItemType Directory -Force -Path $llvm18Dir | Out-Null
                Copy-Item -Path $llvmDir -Destination (Join-Path $llvm18Dir "llvm") -Recurse -Force -ErrorAction SilentlyContinue
            }
            $llvmC18Dir = Join-Path $ClangdIncludesDir "llvm-c-18"
            if (-not (Test-Path $llvmC18Dir)) {
                New-Item -ItemType Directory -Force -Path $llvmC18Dir | Out-Null
                Copy-Item -Path $llvmCDir -Destination (Join-Path $llvmC18Dir "llvm-c") -Recurse -Force -ErrorAction SilentlyContinue
            }
        }
    } catch {}
}

# Normalize path separators for container arguments (convert Windows \ to Linux /)
$NormalizedArgs = @()
foreach ($arg in $rawArgs) {
    if (($arg -match '^[a-zA-Z]:\\' -or $arg -like "*\*") -and -not ($arg -like "*`$*")) {
        $NormalizedArgs += $arg.Replace("\", "/")
    } else {
        $NormalizedArgs += $arg
    }
}

# Execute Docker command
& docker.exe @NormalizedArgs
$exitCode = $LASTEXITCODE
if ($null -eq $exitCode) { $exitCode = 0 }

# Use the same structural JSON adaptation as Linux.
if ($exitCode -eq 0 -and $rawArgs.Count -gt 0 -and $rawArgs[0] -eq "run") {
    & python (Join-Path $ScriptDir "../compile_commands.py") $RootDir
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
exit $exitCode
