# Organize Scripts in Telemetry-Platform
# Moves all .ps1 scripts to scripts/ folder for cleaner repository structure

cd C:\code\telemetry-platform

Write-Host "=== Organizing Telemetry-Platform Scripts ===" -ForegroundColor Cyan
Write-Host ""

# Create scripts directory if it doesn't exist
if (-not (Test-Path "scripts")) {
    New-Item -ItemType Directory -Path "scripts" -Force | Out-Null
    Write-Host "✓ Created scripts/ directory" -ForegroundColor Green
} else {
    Write-Host "✓ scripts/ directory already exists" -ForegroundColor Green
}

Write-Host ""
Write-Host "Moving PowerShell scripts from root to scripts/..." -ForegroundColor Yellow
Write-Host ""

# List of scripts to move from root
$scriptsToMove = @(
    "build_with_gui.ps1",
    "build.ps1",
    "run_grafana_test.ps1",
    "run_load_test.ps1",
    "setup_grafana_k6.ps1"
)

$movedCount = 0
$notFoundCount = 0

foreach ($script in $scriptsToMove) {
    if (Test-Path $script) {
        Move-Item $script "scripts/" -Force
        Write-Host "  ✓ Moved: $script" -ForegroundColor Green
        $movedCount++
    } else {
        Write-Host "  ⊘ Not found: $script (may already be moved)" -ForegroundColor Gray
        $notFoundCount++
    }
}

Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host "✓ Moved: $movedCount scripts" -ForegroundColor Green
Write-Host "⊘ Not found: $notFoundCount scripts" -ForegroundColor Gray
Write-Host ""
Write-Host "Repository structure cleaned! Check: C:\code\telemetry-platform\scripts\" -ForegroundColor Cyan
Write-Host ""

# Show contents of scripts folder
Write-Host "Contents of scripts/ folder:" -ForegroundColor Yellow
Get-ChildItem scripts\*.ps1 | Select-Object Name | Format-Table -HideTableHeaders
