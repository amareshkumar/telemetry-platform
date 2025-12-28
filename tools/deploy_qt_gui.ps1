# Qt GUI Deployment Script
# Copies all necessary Qt DLLs and plugins for gui_app.exe

$QtPath = "C:\Qt\6.10.1\msvc2022_64"
$GuiRelease = "C:\code\telemetry-platform\build_clean\ingestion\gui\Release"

Write-Host "Deploying Qt dependencies to GUI application..." -ForegroundColor Cyan

# Copy Qt Core DLLs
Write-Host "Copying Qt DLLs..." -ForegroundColor Yellow
Copy-Item "$QtPath\bin\Qt6Core.dll" $GuiRelease -Force
Copy-Item "$QtPath\bin\Qt6Gui.dll" $GuiRelease -Force
Copy-Item "$QtPath\bin\Qt6Widgets.dll" $GuiRelease -Force
Write-Host "  Done: 3 DLLs copied" -ForegroundColor Green

# Copy platform plugin
Write-Host "Copying Qt platform plugins..." -ForegroundColor Yellow
$platformsDir = Join-Path $GuiRelease "platforms"
if (!(Test-Path $platformsDir)) {
    New-Item -ItemType Directory -Path $platformsDir -Force | Out-Null
}
Copy-Item "$QtPath\plugins\platforms\qwindows.dll" $platformsDir -Force
Write-Host "  Done: qwindows.dll copied" -ForegroundColor Green

Write-Host "`nDeployment complete!" -ForegroundColor Green
Write-Host "Run: cd $GuiRelease and then gui_app.exe" -ForegroundColor Cyan

