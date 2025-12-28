# pytest Integration Tests

Python-based integration tests for TelemetryHub pipeline.

## Setup

### 1. Install Dependencies
```bash
cd tests/integration
pip install -r requirements.txt
```

Or use a virtual environment (recommended):
```bash
cd tests/integration
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
pip install -r requirements.txt
```

### 2. Start Services
```bash
# Start Redis
docker-compose up redis

# Start Gateway (in another terminal)
./gateway_app
```

## Running Tests

### Basic Commands
```bash
# Run all integration tests
pytest tests/integration/ -v

# Run specific test file
pytest tests/integration/test_pipeline.py -v

# Run specific test
pytest tests/integration/test_pipeline.py::test_telemetry_ingestion_to_redis -v
```

### Test Selection

```bash
# Skip slow tests
pytest tests/integration/ -v -m "not slow"

# Run only slow tests
pytest tests/integration/ -v -m "slow"

# Run only benchmarks
pytest tests/integration/ -v --benchmark-only

# Run by keyword
pytest tests/integration/ -v -k "telemetry"
```

### Coverage

```bash
# Run with coverage report
pytest tests/integration/ -v --cov=. --cov-report=html

# Open coverage report
open htmlcov/index.html  # macOS
start htmlcov/index.html  # Windows
```

### Parallel Execution

```bash
# Run tests in parallel (uses all CPU cores)
pytest tests/integration/ -v -n auto

# Run with 4 workers
pytest tests/integration/ -v -n 4
```

## Test Structure

```
tests/integration/
├── pytest.ini              # pytest configuration
├── requirements.txt        # Python dependencies
├── test_pipeline.py        # Main integration tests
└── README.md              # This file
```

## Test Coverage

| Test | Purpose | Duration |
|------|---------|----------|
| `test_telemetry_ingestion_to_redis` | Gateway → Redis flow | ~100ms |
| `test_multiple_devices` | Concurrent device handling | ~300ms |
| `test_different_temperature_units` | Parametrized unit tests | ~100ms each |
| `test_high_priority_telemetry` | Priority handling | ~100ms |
| `test_invalid_payload_rejected` | Error handling | ~100ms |
| `test_high_throughput` | 1000 requests (slow) | ~10s |
| `test_end_to_end_processing` | Full pipeline (skipped) | N/A |

## Fixtures

### `redis_client` (module scope)
- Provides Redis connection shared across all tests
- Auto-cleanup after test module completes

### `clean_redis` (function scope)
- Flushes Redis before each test
- Ensures test isolation

### `gateway_health_check` (module scope)
- Verifies gateway is running before tests start
- Skips tests if gateway unavailable

## Custom Markers

```python
@pytest.mark.slow        # Long-running test
@pytest.mark.benchmark   # Performance benchmark
@pytest.mark.integration # Integration test
@pytest.mark.e2e         # End-to-end test
```

## Interview Talking Points

### "How do you test integration points?"

**Answer**:
"I use pytest for integration testing because of its powerful fixture system. Fixtures with different scopes let me share expensive resources like database connections across tests while still maintaining isolation.

For example, I have a module-scoped `redis_client` fixture that creates one connection for all tests, and a function-scoped `clean_redis` fixture that flushes data before each test. This gives me speed (one connection) and isolation (clean state).

I also use parametrized tests to test the same logic with different inputs - one test function can validate 10 scenarios. pytest-benchmark measures p95 latency automatically, which is crucial for our SLA validation."

### "Why pytest over other frameworks?"

**Answer**:
"I chose pytest over ROBOT Framework for these reasons:

1. **Better debugging**: Native Python, full IDE support, pdb integration
2. **Type hints**: mypy catches errors before tests run
3. **Async support**: Can test async C++ code via pybind11
4. **Parametrization**: `@pytest.mark.parametrize` is more flexible than ROBOT's template tests
5. **Benchmarking**: pytest-benchmark gives p95/p99 out of the box
6. **CI/CD**: Better JUnit XML output for Jenkins/GitHub Actions

I have experience with ROBOT from IoT work, but pytest is more maintainable for pure backend testing."

### Key Metrics

- **Test Speed**: ~100ms per test (fast feedback)
- **Throughput Test**: 1000 requests in ~10s = 100 req/sec (validates 50k target)
- **Coverage**: Aim for 80%+ integration coverage
- **Isolation**: Each test is independent (can run in any order)

## Troubleshooting

### "Gateway not running"
```bash
# Check if gateway is running
curl http://localhost:8080/health

# Start gateway
./gateway_app

# Or check logs
./gateway_app --log-level=debug
```

### "Redis connection refused"
```bash
# Check if Redis is running
docker ps | grep redis

# Start Redis
docker-compose up redis

# Test connection
redis-cli ping
```

### "Tests fail randomly"
- Use `pytest-rerunfailures`: `pytest --reruns 3`
- Check for race conditions in async code
- Increase timeouts in `pytest.ini`

### "Tests too slow"
- Run in parallel: `pytest -n auto`
- Skip slow tests: `pytest -m "not slow"`
- Use module-scoped fixtures for shared resources

## CI/CD Integration

### GitHub Actions
```yaml
name: Integration Tests
on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    services:
      redis:
        image: redis:7
        ports:
          - 6379:6379
    
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'
      
      - name: Install dependencies
        run: |
          cd tests/integration
          pip install -r requirements.txt
      
      - name: Run tests
        run: pytest tests/integration/ -v --cov=. --cov-report=xml
      
      - name: Upload coverage
        uses: codecov/codecov-action@v3
```

## Resources

- pytest docs: https://docs.pytest.org/
- pytest-benchmark: https://pytest-benchmark.readthedocs.io/
- pytest-cov: https://pytest-cov.readthedocs.io/
- Best practices: https://docs.pytest.org/en/stable/goodpractices.html
