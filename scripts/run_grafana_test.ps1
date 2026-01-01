# Run k6 with CSV output for Grafana import
# CSV can be imported into InfluxDB manually

Write-Host "Running k6 test with CSV output..." -ForegroundColor Cyan
Write-Host ""

# Create output directory
$outputDir = "test_results"
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir | Out-Null
}

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$csvFile = "$outputDir/k6_results_$timestamp.csv"
$jsonFile = "$outputDir/k6_results_$timestamp.json"

# Run test with multiple outputs
k6 run `
    --out csv=$csvFile `
    --out json=$jsonFile `
    tests/load/grafana_test.js

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Results saved:" -ForegroundColor Green
Write-Host "  CSV:  $csvFile" -ForegroundColor Yellow
Write-Host "  JSON: $jsonFile" -ForegroundColor Yellow
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "To view results in Grafana:" -ForegroundColor Cyan
Write-Host "1. Open http://localhost:3000" -ForegroundColor White
Write-Host "2. Import dashboard ID: 2587 (k6 Load Testing)" -ForegroundColor White
Write-Host "3. Or manually import the CSV into InfluxDB:" -ForegroundColor White
Write-Host "   docker exec -it telemetry_influxdb influx write \\" -ForegroundColor Gray
Write-Host "     -b k6 -o telemetryhub -t telemetry-admin-token \\" -ForegroundColor Gray
Write-Host "     -f /path/to/$csvFile" -ForegroundColor Gray
