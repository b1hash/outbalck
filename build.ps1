# SimpleRemoter Build Script
# Usage: .\build.ps1 [-Config Release|Debug] [-Platform x64|x86|all] [-ServerOnly] [-Clean] [-Publish]
#
# Requirements:
#   - Visual Studio 2019 or 2022
#   - v142 toolset (MSVC v142 - VS 2019 C++ build tools)
#   - MFC libraries (C++ MFC for v142 build tools)

param(
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release",

    [ValidateSet("x64", "x86", "all")]
    [string]$Platform = "x64",

    [switch]$ServerOnly,  # Only build main server (Yama), skip client projects
    [switch]$Clean,       # Clean before build
    [switch]$Publish      # Publish mode: rebuild all deps + x64 Release + UPX compress
)

$ErrorActionPreference = "Stop"

# Find MSBuild (VS2019 or VS2022, including Insiders/Preview)
# Order: Prefer installations with v142 toolset (VS2019) over VS2022 BuildTools
$msBuildPaths = @(
    # VS2019 Insiders/Preview (non-standard path) - has v142 + MFC
    "${env:ProgramFiles}\Microsoft Visual Studio\18\*\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\18\*\MSBuild\Current\Bin\MSBuild.exe",
    # VS2019 standard
    "${env:ProgramFiles}\Microsoft Visual Studio\2019\*\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\*\MSBuild\Current\Bin\MSBuild.exe",
    # VS2022 (may need v142 toolset installed separately)
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\*\MSBuild\Current\Bin\MSBuild.exe",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\*\MSBuild\Current\Bin\MSBuild.exe"
)

$msBuild = $null
foreach ($pattern in $msBuildPaths) {
    $found = Get-ChildItem $pattern -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        $msBuild = $found.FullName
        break
    }
}

if (-not $msBuild) {
    Write-Host "ERROR: MSBuild not found." -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install Visual Studio 2019 or 2022 with:" -ForegroundColor Yellow
    Write-Host "  - Desktop development with C++" -ForegroundColor White
    Write-Host "  - MSVC v142 - VS 2019 C++ x64/x86 build tools" -ForegroundColor White
    Write-Host "  - C++ MFC for v142 build tools" -ForegroundColor White
    exit 1
}

# Detect VS version
$vsYear = "2019"
if ($msBuild -match "\\2022\\") { $vsYear = "2022" }
elseif ($msBuild -match "\\18\\") { $vsYear = "2019 Insiders" }

Write-Host "Using MSBuild: $msBuild" -ForegroundColor Cyan

$rootDir = $PSScriptRoot
$slnFile = Join-Path $rootDir "2019Remote.sln"
$upxPath = Join-Path $rootDir "server\2015Remote\res\3rd\upx.exe"

# Publish mode overrides
if ($Publish) {
    $Config = "Release"
    $Platform = "x64"
    $ServerOnly = $false
    $Clean = $true
}

# Build function
function Build-Project {
    param(
        [string]$Project,
        [string]$Configuration,
        [string]$Platform,
        [bool]$ForceClean = $false
    )

    # Solution uses x86/x64, projects use Win32/x64 - MSBuild needs solution platform names
    $platArg = $Platform

    Write-Host ""
    Write-Host "Building: $Project | $Configuration | $Platform" -ForegroundColor Yellow

    $buildArgs = @(
        $slnFile,
        "/t:$Project",
        "/p:Configuration=$Configuration",
        "/p:Platform=$platArg",
        "/m",           # Parallel build
        "/v:minimal"    # Minimal verbosity
    )

    if ($Clean -or $ForceClean) {
        # Use :Rebuild target for single project rebuild
        $buildArgs[1] = "/t:${Project}:Rebuild"
    }

    & $msBuild $buildArgs

    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Build failed for $Project" -ForegroundColor Red
        Write-Host ""
        Write-Host "If you see 'v142 build tools cannot be found':" -ForegroundColor Yellow
        Write-Host "  Install 'MSVC v142 - VS 2019 C++ build tools' via VS Installer" -ForegroundColor White
        Write-Host ""
        Write-Host "If you see 'MFC libraries are required':" -ForegroundColor Yellow
        Write-Host "  Install 'C++ MFC for v142 build tools' via VS Installer" -ForegroundColor White
        exit 1
    }

    Write-Host "OK: $Project" -ForegroundColor Green
}

# Determine platforms to build
$platforms = if ($Platform -eq "all") { @("x86", "x64") } else { @($Platform) }

# Client projects (dependencies) - must build both x86 and x64 for embedding
$clientProjects = @(
    "ServerDll",    # ClientDll_vs2015.vcxproj -> ServerDll.dll
    "ghost",        # ghost_vs2015.vcxproj -> ghost.exe
    "TestRun",      # TestRun_vs2015.vcxproj -> TestRun.exe
    "TinyRun",      # TinyRun.vcxproj -> TinyRun.dll
    "SCLoader"      # SCLoader.vcxproj -> SCLoader.exe
)

# Server project
$serverProject = "Yama"  # 2015Remote_vs2015.vcxproj -> Yama.exe

$startTime = Get-Date

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
if ($Publish) {
    Write-Host " SimpleRemoter PUBLISH Build" -ForegroundColor Cyan
} else {
    Write-Host " SimpleRemoter Build" -ForegroundColor Cyan
}
Write-Host " Config: $Config" -ForegroundColor Cyan
Write-Host " Platform: $Platform" -ForegroundColor Cyan
Write-Host " VS: $vsYear" -ForegroundColor Cyan
Write-Host " Clean: $Clean" -ForegroundColor Cyan
if ($Publish) {
    Write-Host " UPX: Yes" -ForegroundColor Cyan
}
Write-Host "========================================" -ForegroundColor Cyan

if (-not $ServerOnly) {
    Write-Host ""
    Write-Host "Step 1: Building client projects (Release x86 + x64)..." -ForegroundColor Magenta
    Write-Host "  (Always Release - embedded in server resources)" -ForegroundColor DarkGray

    # Client projects are always built as Release (embedded in server resources)
    # In Publish mode, force clean rebuild
    foreach ($proj in $clientProjects) {
        Build-Project -Project $proj -Configuration "Release" -Platform "x86" -ForceClean $Publish
        Build-Project -Project $proj -Configuration "Release" -Platform "x64" -ForceClean $Publish
    }
}

Write-Host ""
Write-Host "Step 2: Building server (Yama)..." -ForegroundColor Magenta

foreach ($plat in $platforms) {
    Build-Project -Project $serverProject -Configuration $Config -Platform $plat -ForceClean $Publish
}

# UPX compression in Publish mode
$finalOutput = $null
if ($Publish) {
    Write-Host ""
    Write-Host "Step 3: UPX compression..." -ForegroundColor Magenta

    $binPath = Join-Path (Join-Path $rootDir "Bin") "Yama_x64.exe"

    if (-not (Test-Path $upxPath)) {
        Write-Host "ERROR: UPX not found at $upxPath" -ForegroundColor Red
        exit 1
    }

    if (-not (Test-Path $binPath)) {
        Write-Host "ERROR: Output file not found at $binPath" -ForegroundColor Red
        exit 1
    }

    $sizeBefore = (Get-Item $binPath).Length / 1MB
    Write-Host "  Before: $($sizeBefore.ToString('F2')) MB" -ForegroundColor DarkGray

    & $upxPath --best $binPath

    if ($LASTEXITCODE -ne 0) {
        Write-Host "WARNING: UPX compression failed, but build succeeded" -ForegroundColor Yellow
    } else {
        $sizeAfter = (Get-Item $binPath).Length / 1MB
        $ratio = (1 - $sizeAfter / $sizeBefore) * 100
        Write-Host "  After:  $($sizeAfter.ToString('F2')) MB (-$($ratio.ToString('F1'))%)" -ForegroundColor Green
    }

    $finalOutput = $binPath
}

$elapsed = (Get-Date) - $startTime

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
if ($Publish) {
    Write-Host " Publish Completed!" -ForegroundColor Green
} else {
    Write-Host " Build Completed!" -ForegroundColor Green
}
Write-Host " Time: $($elapsed.TotalSeconds.ToString('F1')) seconds" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

# Show output files
Write-Host ""
Write-Host "Output files:" -ForegroundColor Cyan

if ($finalOutput) {
    $size = (Get-Item $finalOutput).Length / 1MB
    Write-Host "  $finalOutput ($($size.ToString('F2')) MB)" -ForegroundColor White
} else {
    foreach ($plat in $platforms) {
        # Check both possible output locations (project has custom OutputFile)
        $suffix = if ($plat -eq "x86") { "_x86" } else { "_x64" }
        if ($Config -eq "Debug") { $suffix += "d" }
        $binPath = Join-Path (Join-Path $rootDir "Bin") "Yama$suffix.exe"

        $outDir = if ($plat -eq "x86") { "Release" } else { "x64\Release" }
        if ($Config -eq "Debug") {
            $outDir = if ($plat -eq "x86") { "Debug" } else { "x64\Debug" }
        }
        $stdPath = Join-Path (Join-Path $rootDir $outDir) "Yama.exe"

        # Prefer Bin output, fallback to standard path
        $outPath = if (Test-Path $binPath) { $binPath } elseif (Test-Path $stdPath) { $stdPath } else { $null }

        if ($outPath) {
            $size = (Get-Item $outPath).Length / 1MB
            Write-Host "  $outPath ($($size.ToString('F2')) MB)" -ForegroundColor White
        }
    }
}
