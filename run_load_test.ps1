# TelemetryHub Load Test Runner
# Starts gateway and runs k6 test

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "TelemetryHub Load Test Runner" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Check if gateway is already running
$gatewayProcess = Get-Process -Name "gateway_app" -ErrorAction SilentlyContinue
if ($gatewayProcess) {
    Write-Host "✓ Gateway already running (PID: $($gatewayProcess.Id))" -ForegroundColor Green
} else {
    Write-Host "Starting gateway..." -ForegroundColor Yellow
    Start-Process powershell -ArgumentList "-NoExit", "-Command", "cd '$PSScriptRoot'; Write-Host 'Gateway Server' -ForegroundColor Green; .\build_simple\ingestion\gateway\Release\gateway_app.exe"
    
    Write-Host "Waiting for gateway to start..." -ForegroundColor Yellow
    Start-Sleep -Seconds 3
    
    # Verify gateway is responding
    try {
        $health = Invoke-WebRequest -Uri http://localhost:8080/health -UseBasicParsing -TimeoutSec 5
        Write-Host "✓ Gateway health check: $($health.StatusCode)" -ForegroundColor Green
    } catch {
        Write-Host "✗ Gateway failed to start!" -ForegroundColor Red
        Write-Host "Error: $_" -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Running k6 Load Test" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Run k6 test
k6 run tests/load/simple_test.js

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "Test Complete" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
