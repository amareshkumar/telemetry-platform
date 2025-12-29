# Setup Grafana + k6 Integration
# Installs xk6-output-influxdb extension for native InfluxDB v2 support

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Grafana + k6 Integration Setup" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Check if Go is installed (required for xk6)
$goInstalled = Get-Command go -ErrorAction SilentlyContinue
if (!$goInstalled) {
    Write-Host "Go is not installed. Installing via Chocolatey..." -ForegroundColor Yellow
    choco install golang -y
    refreshenv
}

# Install xk6 (k6 extension builder)
Write-Host "Installing xk6..." -ForegroundColor Yellow
go install go.k6.io/xk6/cmd/xk6@latest

# Build k6 with InfluxDB v2 extension
Write-Host "Building k6 with InfluxDB v2 extension..." -ForegroundColor Yellow
$xk6Path = "$env:USERPROFILE\go\bin\xk6.exe"
if (Test-Path $xk6Path) {
    & $xk6Path build --with github.com/grafana/xk6-output-influxdb
    Write-Host "✓ k6 built with InfluxDB v2 support" -ForegroundColor Green
} else {
    Write-Host "✗ xk6 not found. Install manually:" -ForegroundColor Red
    Write-Host "  go install go.k6.io/xk6/cmd/xk6@latest" -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Alternative: Use Prometheus + Grafana" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. Export k6 metrics to Prometheus format:" -ForegroundColor White
Write-Host "   k6 run --out experimental-prometheus-rw tests/load/grafana_test.js" -ForegroundColor Gray
Write-Host ""
Write-Host "2. Or use JSON output and visualize with external tools:" -ForegroundColor White
Write-Host "   k6 run --out json=results.json tests/load/grafana_test.js" -ForegroundColor Gray
Write-Host "   Then import JSON into Grafana using JSON API data source" -ForegroundColor Gray
Write-Host ""
Write-Host "3. Current working solution: CSV export" -ForegroundColor White
Write-Host "   Results are in test_results/ directory" -ForegroundColor Gray
Write-Host "   Use .\run_grafana_test.ps1 to generate CSV/JSON" -ForegroundColor Gray
