# docker/environnement_dev/windows/docker-cli.ps1 - Transparent Docker runner and Clangd indexer for Windows 10/11
$ErrorActionPreference = "Continue"

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

Ensure-DockerDaemon

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
$ForwardRootDir = $RootDir.Replace("\", "/")

# Ensure persistent ccache volume exists with write permissions
& docker.exe volume inspect prysma-ccache 2>$null | Out-Null
if ($LASTEXITCODE -ne 0) {
    & docker.exe volume create prysma-ccache 2>$null | Out-Null
    & docker.exe run --rm -v prysma-ccache:/tmp/.ccache prysma-debug chmod 777 /tmp/.ccache 2>$null | Out-Null
}

# Ensure .clangd-includes is populated for host Clangd indexing
$ClangdIncludesDir = Join-Path $RootDir ".clangd-includes"
if (-not (Test-Path (Join-Path $ClangdIncludesDir "llvm\Support\TargetSelect.h"))) {
    try {
        $tempContainer = (& docker.exe create prysma-debug 2>$null)
        if ($tempContainer -and $LASTEXITCODE -eq 0) {
            $tempContainer = $tempContainer.Trim()
            New-Item -ItemType Directory -Force -Path $ClangdIncludesDir | Out-Null
            & docker.exe cp "${tempContainer}:/usr/lib/llvm-18/include/." "$ClangdIncludesDir" 2>$null | Out-Null
            & docker.exe rm -f $tempContainer 2>$null | Out-Null
            $llvm18Dir = Join-Path $ClangdIncludesDir "llvm-18"
            if (-not (Test-Path $llvm18Dir)) {
                New-Item -ItemType Directory -Force -Path $llvm18Dir | Out-Null
                Copy-Item -Path (Join-Path $ClangdIncludesDir "llvm") -Destination (Join-Path $llvm18Dir "llvm") -Recurse -Force -ErrorAction SilentlyContinue
                Copy-Item -Path (Join-Path $ClangdIncludesDir "llvm-c") -Destination (Join-Path $llvm18Dir "llvm-c") -Recurse -Force -ErrorAction SilentlyContinue
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

# UTF-8 without BOM encoder for Clangd compatibility
$utf8NoBom = New-Object System.Text.UTF8Encoding $false

# Update compile_commands.json for Clangd on the host
$cdbFiles = @(
    (Join-Path $RootDir "build\compile_commands.json"),
    (Join-Path $RootDir "tests\unit_tests\build\compile_commands.json")
)

foreach ($cdb in $cdbFiles) {
    if (Test-Path $cdb) {
        try {
            $content = [System.IO.File]::ReadAllText($cdb)
            $modified = $false
            if ($content.Contains("/workspace")) {
                $content = $content.Replace("/workspace", $ForwardRootDir)
                $modified = $true
            }
            if ($content.Contains("/home/zyph/prysma")) {
                $content = $content.Replace("/home/zyph/prysma", $ForwardRootDir)
                $modified = $true
            }
            if ($content.Contains("/usr/lib/llvm-18/include")) {
                $content = $content.Replace("/usr/lib/llvm-18/include", "$ForwardRootDir/.clangd-includes")
                $modified = $true
            }
            if ($modified) {
                [System.IO.File]::WriteAllText($cdb, $content, $utf8NoBom)
            }
        } catch {}
    }
}

# Merge compile_commands.json if both exist
$mainCdb = Join-Path $RootDir "build\compile_commands.json"
$testsCdb = Join-Path $RootDir "tests\unit_tests\build\compile_commands.json"
$rootCdb = Join-Path $RootDir "compile_commands.json"

if ((Test-Path $mainCdb) -and (Test-Path $testsCdb)) {
    try {
        $jsonMain = Get-Content $mainCdb -Raw | ConvertFrom-Json
        $jsonTests = Get-Content $testsCdb -Raw | ConvertFrom-Json
        $files = @{}
        foreach ($e in $jsonMain) { $files[$e.file] = $true }
        $merged = [System.Collections.ArrayList]@($jsonMain)
        foreach ($e in $jsonTests) {
            if (-not $files.ContainsKey($e.file)) {
                [void]$merged.Add($e)
            }
        }
        $mergedJson = $merged | ConvertTo-Json -Depth 10
        [System.IO.File]::WriteAllText($mainCdb, $mergedJson, $utf8NoBom)
        [System.IO.File]::WriteAllText($rootCdb, $mergedJson, $utf8NoBom)
    } catch {}
}

exit $exitCode
