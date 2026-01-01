#!/usr/bin/env pwsh
<#
.SYNOPSIS
Build TelemetryPlatform with GUI support using VS 2026 Preview

.DESCRIPTION
Configures and builds the project with:
- BUILD_GUI=ON (Qt-based telemetry viewer)
- BUILD_TESTS=ON (all unit/integration tests)
- Visual Studio 2026 Preview
- Release configuration

.PARAMETER Clean
If specified, removes build directory before configuring

.EXAMPLE
.\build_with_gui.ps1
.\build_with_gui.ps1 -Clean
#>

param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Project paths
$ProjectRoot = $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build_vs26"
$LogFile = Join-Path $ProjectRoot "build_gui.txt"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "TelemetryPlatform Build with GUI" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Clean build directory if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "[1/4] Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Path $BuildDir -Recurse -Force
    Write-Host "      ✓ Build directory removed" -ForegroundColor Green
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    Write-Host "[1/4] Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
    Write-Host "      ✓ Directory created: $BuildDir" -ForegroundColor Green
} else {
    Write-Host "[1/4] Build directory exists" -ForegroundColor Green
}

# Change to build directory
Push-Location $BuildDir

try {
    # CMake Configuration
    Write-Host ""
    Write-Host "[2/4] Configuring CMake..." -ForegroundColor Yellow
    Write-Host "      - Generator: Visual Studio 18 2026" -ForegroundColor Gray
    Write-Host "      - BUILD_GUI: ON" -ForegroundColor Gray
    Write-Host "      - BUILD_TESTS: ON" -ForegroundColor Gray
    Write-Host ""
    
    $ConfigStart = Get-Date
    
    # Run CMake configure
    & cmake .. `
        -G "Visual Studio 18 2026" `
        -A x64 `
        -DBUILD_GUI=ON `
        -DBUILD_TESTS=ON
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "✗ CMake configuration FAILED" -ForegroundColor Red
        Write-Host "  Check for Qt installation or dependency issues" -ForegroundColor Yellow
        exit 1
    }
    
    $ConfigTime = (Get-Date) - $ConfigStart
    Write-Host ""
    Write-Host "      ✓ Configuration complete ($([int]$ConfigTime.TotalSeconds)s)" -ForegroundColor Green
    
    # CMake Build
    Write-Host ""
    Write-Host "[3/4] Building project (Release)..." -ForegroundColor Yellow
    Write-Host "      This may take 5-10 minutes on first build..." -ForegroundColor Gray
    Write-Host ""
    
    $BuildStart = Get-Date
    
    # Build with detailed logging
    & cmake --build . --config Release 2>&1 | Tee-Object -FilePath $LogFile
    
    $BuildCode = $LASTEXITCODE
    $BuildTime = (Get-Date) - $BuildStart
    
    Write-Host ""
    if ($BuildCode -eq 0) {
        Write-Host "      ✓ Build SUCCEEDED ($([int]$BuildTime.TotalSeconds)s)" -ForegroundColor Green
    } else {
        Write-Host "      ✗ Build FAILED (Exit code: $BuildCode)" -ForegroundColor Red
        Write-Host "      Log saved to: $LogFile" -ForegroundColor Yellow
        exit $BuildCode
    }
    
    # Run Tests
    Write-Host ""
    Write-Host "[4/4] Running tests..." -ForegroundColor Yellow
    
    $TestStart = Get-Date
    & ctest -C Release --output-on-failure
    $TestCode = $LASTEXITCODE
    $TestTime = (Get-Date) - $TestStart
    
    Write-Host ""
    if ($TestCode -eq 0) {
        Write-Host "      ✓ All tests PASSED ($([int]$TestTime.TotalSeconds)s)" -ForegroundColor Green
    } else {
        Write-Host "      ✗ Some tests FAILED (Exit code: $TestCode)" -ForegroundColor Red
    }
    
    # Summary
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Build Summary" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Configure: $([int]$ConfigTime.TotalSeconds)s" -ForegroundColor White
    Write-Host "Build:     $([int]$BuildTime.TotalSeconds)s" -ForegroundColor White
    Write-Host "Test:      $([int]$TestTime.TotalSeconds)s" -ForegroundColor White
    Write-Host "Total:     $([int](($ConfigTime + $BuildTime + $TestTime).TotalSeconds))s" -ForegroundColor White
    Write-Host ""
    
    # Check for GUI executable
    $GuiExe = Join-Path $BuildDir "ingestion\gui\Release\telemetry_gui.exe"
    if (Test-Path $GuiExe) {
        Write-Host "✓ GUI executable built: $GuiExe" -ForegroundColor Green
    } else {
        Write-Host "⚠ GUI executable not found (Qt may not be installed)" -ForegroundColor Yellow
        Write-Host "  Expected: $GuiExe" -ForegroundColor Gray
    }
    
    Write-Host ""
    Write-Host "Build log: $LogFile" -ForegroundColor Gray
    
    exit $BuildCode
    
} finally {
    Pop-Location
}
