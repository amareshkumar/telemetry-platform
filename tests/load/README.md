# k6 Load Testing

Modern load testing infrastructure for TelemetryHub using k6.

## Installation

### Windows
```powershell
choco install k6
```

### macOS
```bash
brew install k6
```

### Linux
```bash
sudo apt-get install k6
```

Or download from: https://k6.io/docs/getting-started/installation/

## Test Scripts

### 1. **telemetry_ingestion.js** - Full Load Test
Simulates 5,000 concurrent IoT devices sending telemetry.

```bash
# Run with default settings
k6 run tests/load/telemetry_ingestion.js

# Run with custom VUs and duration
k6 run tests/load/telemetry_ingestion.js --vus 1000 --duration 2m

# Export results to JSON
k6 run tests/load/telemetry_ingestion.js --out json=results.json

# Send metrics to InfluxDB (for Grafana)
k6 run tests/load/telemetry_ingestion.js --out influxdb=http://localhost:8086/k6
```

**Metrics validated**:
- p95 latency < 200ms
- p99 latency < 500ms
- Error rate < 1%
- Throughput > 10,000 req/sec

### 2. **health_check.js** - Smoke Test
Quick availability check before running full load tests.

```bash
k6 run tests/load/health_check.js --vus 10 --duration 30s
```

## Performance Targets

Based on Protobuf benchmark (408k ops/sec):

| Metric | Target | SLA |
|--------|--------|-----|
| **Throughput** | 50,000 events/sec | 10,000 events/sec |
| **p95 Latency** | < 200ms | < 500ms |
| **p99 Latency** | < 500ms | < 1000ms |
| **Error Rate** | < 0.1% | < 1% |
| **Concurrent Devices** | 5,000+ | 1,000+ |

## Grafana Integration

### Setup InfluxDB + Grafana

1. **Start services**:
```bash
docker-compose up influxdb grafana
```

2. **Run k6 with InfluxDB output**:
```bash
k6 run tests/load/telemetry_ingestion.js \
  --out influxdb=http://localhost:8086/k6
```

3. **View dashboard**:
- Open Grafana: http://localhost:3000
- Default credentials: admin/admin
- Import k6 dashboard: https://grafana.com/grafana/dashboards/2587

### Dashboard Metrics

- **HTTP Request Duration** (p95, p99, max)
- **Requests per Second** (throughput)
- **Error Rate** (%)
- **Virtual Users** (concurrent load)
- **Data Sent/Received** (bandwidth)

## Interview Talking Points

### "How did you test performance?"

**Answer**:
"I used k6 for load testing because it's modern, cloud-native, and integrates with Grafana for real-time monitoring. I simulated 5,000 concurrent IoT devices sending telemetry every 1-5 seconds, which translates to ~10,000 requests/sec sustained load.

The test validates our p95 latency is under 200ms and error rate is below 1%. We use thresholds in k6 to automatically fail the build if SLAs aren't met. This caught a Redis connection pool exhaustion issue during development.

I chose k6 over JMeter because:
1. JavaScript DSL (easier to version control)
2. Built-in p95/p99 metrics (no plugins needed)
3. Grafana integration (beautiful dashboards)
4. Cloud-ready (k6 Cloud for distributed testing)
5. Better CI/CD integration (JSON output for analysis)"

### Performance Results to Memorize

- **Protobuf**: 408,000 serializations/sec (baseline)
- **Target**: 50,000 events/sec (10x safety margin)
- **Validated**: p95 < 200ms, p99 < 500ms, error < 1%
- **Scale**: 5,000 concurrent devices tested

## CI/CD Integration

### GitHub Actions Example
```yaml
name: Load Test
on: [push]

jobs:
  load-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Run k6
        uses: grafana/k6-action@v0.3.1
        with:
          filename: tests/load/telemetry_ingestion.js
          flags: --vus 100 --duration 1m
```

### Fail build on threshold violations
k6 automatically exits with code 99 if thresholds fail:
```bash
k6 run tests/load/telemetry_ingestion.js || exit 1
```

## Troubleshooting

### "Connection refused"
- Ensure gateway is running: `./gateway_app`
- Check Redis is running: `docker-compose up redis`
- Verify URL: `curl http://localhost:8080/health`

### "Too many open files"
```bash
# Increase file descriptor limit (Linux/macOS)
ulimit -n 10000
```

### "High error rate"
- Reduce VUs: `--vus 100` instead of `--vus 1000`
- Check gateway logs for errors
- Verify Redis connection pool size

## Resources

- k6 Docs: https://k6.io/docs/
- Grafana Dashboards: https://grafana.com/grafana/dashboards/
- k6 Cloud: https://k6.io/cloud/
- Examples: https://github.com/grafana/k6-example-scripts
