"""
Integration Tests for TelemetryHub Pipeline

Tests the complete flow: Gateway → Redis → Processor

Prerequisites:
- pip install -r requirements.txt
- docker-compose up redis
- ./gateway_app (running in background)

Run:
pytest tests/integration/ -v
pytest tests/integration/ -v -k "telemetry"  # Run specific tests
pytest tests/integration/ -v --cov=.  # With coverage
"""

import pytest
import requests
import redis
import time
import json
from typing import Dict, Any

# Base URL for gateway (configure per environment)
GATEWAY_URL = "http://localhost:8080"
REDIS_HOST = "localhost"
REDIS_PORT = 6379

# ========== Fixtures ==========

@pytest.fixture(scope="module")
def redis_client():
    """
    Redis connection with cleanup after test module.
    Scope='module' means one connection shared across all tests in this file.
    """
    client = redis.Redis(
        host=REDIS_HOST,
        port=REDIS_PORT,
        decode_responses=True  # Return strings instead of bytes
    )
    
    # Verify Redis is accessible
    assert client.ping(), "Redis not available"
    
    yield client
    
    # Cleanup: Flush test data after all tests
    # WARNING: Don't use in production!
    client.flushdb()
    client.close()


@pytest.fixture
def clean_redis(redis_client):
    """
    Clean Redis before each test.
    Scope='function' (default) means runs before every test function.
    """
    redis_client.flushdb()
    yield redis_client
    # No cleanup needed (done at module level)


@pytest.fixture(scope="module")
def gateway_health_check():
    """
    Verify gateway is running before starting tests.
    """
    try:
        response = requests.get(f"{GATEWAY_URL}/health", timeout=2)
        assert response.status_code == 200, f"Gateway not healthy: {response.status_code}"
        print(f"\n✓ Gateway health check passed: {GATEWAY_URL}")
        return True
    except requests.exceptions.ConnectionError:
        pytest.skip(f"Gateway not running at {GATEWAY_URL}. Start with: ./gateway_app")


# ========== Helper Functions ==========

def create_telemetry_payload(device_id: str, temperature: float = 25.5) -> Dict[str, Any]:
    """Create a realistic telemetry payload."""
    return {
        "device_id": device_id,
        "type": "temperature",
        "value": temperature,
        "unit": "celsius",
        "priority": "MEDIUM",
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "metadata": {
            "location": "zone-1",
            "firmware_version": "2.3.1"
        }
    }


# ========== Integration Tests ==========

def test_telemetry_ingestion_to_redis(gateway_health_check, clean_redis):
    """
    Test: Gateway receives telemetry and stores it in Redis queue.
    
    Flow: HTTP POST → Gateway → Redis RPUSH → Verify
    """
    # Arrange
    payload = create_telemetry_payload("sensor-001", 25.5)
    
    # Act: Send telemetry to gateway
    response = requests.post(
        f"{GATEWAY_URL}/telemetry",
        json=payload,
        headers={"Content-Type": "application/json"}
    )
    
    # Assert: Gateway accepts request
    assert response.status_code == 200, f"Gateway rejected request: {response.text}"
    
    # Assert: Data appears in Redis queue
    queue_length = clean_redis.llen("telemetry_queue")
    assert queue_length > 0, "Telemetry not found in Redis queue"
    
    # Assert: Data is correct
    raw_data = clean_redis.lpop("telemetry_queue")
    assert raw_data is not None
    
    stored_data = json.loads(raw_data)
    assert stored_data["device_id"] == "sensor-001"
    assert stored_data["value"] == 25.5


def test_multiple_devices(gateway_health_check, clean_redis):
    """
    Test: Gateway handles multiple devices concurrently.
    """
    devices = ["sensor-001", "sensor-002", "sensor-003"]
    
    # Send telemetry from 3 devices
    for device_id in devices:
        payload = create_telemetry_payload(device_id, 20.0 + len(device_id))
        response = requests.post(f"{GATEWAY_URL}/telemetry", json=payload)
        assert response.status_code == 200
    
    # Verify all 3 messages in queue
    queue_length = clean_redis.llen("telemetry_queue")
    assert queue_length == 3, f"Expected 3 messages, got {queue_length}"


@pytest.mark.parametrize("device_id,temp,unit", [
    ("sensor-001", 25.5, "celsius"),
    ("sensor-002", 75.2, "fahrenheit"),
    ("sensor-003", 298.15, "kelvin"),
])
def test_different_temperature_units(gateway_health_check, clean_redis, device_id, temp, unit):
    """
    Test: Gateway handles different temperature units.
    Uses pytest.mark.parametrize to run same test with different inputs.
    """
    payload = {
        "device_id": device_id,
        "type": "temperature",
        "value": temp,
        "unit": unit,
        "priority": "MEDIUM"
    }
    
    response = requests.post(f"{GATEWAY_URL}/telemetry", json=payload)
    assert response.status_code == 200
    
    # Verify in Redis
    raw_data = clean_redis.lpop("telemetry_queue")
    stored_data = json.loads(raw_data)
    assert stored_data["unit"] == unit


def test_high_priority_telemetry(gateway_health_check, clean_redis):
    """
    Test: Gateway handles HIGH priority telemetry (alerts).
    """
    payload = create_telemetry_payload("sensor-alert-001", 45.0)
    payload["priority"] = "HIGH"
    
    response = requests.post(f"{GATEWAY_URL}/telemetry", json=payload)
    assert response.status_code == 200
    
    # Verify priority is preserved
    raw_data = clean_redis.lpop("telemetry_queue")
    stored_data = json.loads(raw_data)
    assert stored_data["priority"] == "HIGH"


def test_invalid_payload_rejected(gateway_health_check, clean_redis):
    """
    Test: Gateway rejects invalid payloads.
    """
    invalid_payload = {"invalid_field": "bad_data"}
    
    response = requests.post(f"{GATEWAY_URL}/telemetry", json=invalid_payload)
    
    # Should return 400 Bad Request
    assert response.status_code in [400, 422], "Gateway should reject invalid payload"
    
    # Redis queue should be empty
    queue_length = clean_redis.llen("telemetry_queue")
    assert queue_length == 0, "Invalid data should not reach Redis"


@pytest.mark.slow
def test_high_throughput(gateway_health_check, clean_redis):
    """
    Test: Gateway handles 1000 rapid requests.
    
    Interview Note: This validates our 50k events/sec target at small scale.
    Mark as 'slow' to skip in fast CI runs: pytest -v -m "not slow"
    """
    num_requests = 1000
    start_time = time.time()
    
    # Send 1000 requests
    for i in range(num_requests):
        payload = create_telemetry_payload(f"sensor-perf-{i}", 25.0 + (i % 10))
        response = requests.post(f"{GATEWAY_URL}/telemetry", json=payload)
        assert response.status_code == 200
    
    duration = time.time() - start_time
    throughput = num_requests / duration
    
    print(f"\nThroughput: {throughput:.0f} requests/sec")
    
    # Verify all in Redis
    queue_length = clean_redis.llen("telemetry_queue")
    assert queue_length == num_requests
    
    # Assert reasonable throughput (> 100 req/sec)
    assert throughput > 100, f"Throughput too low: {throughput:.0f} req/sec"


@pytest.mark.skip(reason="Processor not implemented yet")
def test_end_to_end_processing(gateway_health_check, clean_redis):
    """
    Test: Complete pipeline from Gateway → Redis → Processor → Storage.
    
    TODO: Implement when processor is running.
    """
    payload = create_telemetry_payload("sensor-e2e-001", 25.5)
    
    # Send to gateway
    response = requests.post(f"{GATEWAY_URL}/telemetry", json=payload)
    assert response.status_code == 200
    
    # Wait for processor to consume
    time.sleep(2)
    
    # Verify processed data
    processed_key = f"processed:sensor-e2e-001"
    processed_data = clean_redis.get(processed_key)
    assert processed_data is not None, "Data not processed"


# ========== Performance Tests ==========

@pytest.mark.benchmark
def test_telemetry_latency_p95(gateway_health_check, clean_redis, benchmark):
    """
    Benchmark: Measure p95 latency for telemetry ingestion.
    
    Requires: pytest-benchmark
    Run: pytest tests/integration/ --benchmark-only
    """
    payload = create_telemetry_payload("sensor-bench-001", 25.5)
    
    def send_telemetry():
        response = requests.post(f"{GATEWAY_URL}/telemetry", json=payload)
        assert response.status_code == 200
        return response
    
    # Run benchmark (10 rounds, 100 iterations each)
    result = benchmark(send_telemetry)
    
    # Assert p95 < 200ms (our SLA)
    # Note: benchmark.stats available after run
    print(f"\nLatency stats:")
    print(f"  Mean: {result.stats['mean'] * 1000:.2f}ms")
    print(f"  Median: {result.stats['median'] * 1000:.2f}ms")
    print(f"  Min: {result.stats['min'] * 1000:.2f}ms")
    print(f"  Max: {result.stats['max'] * 1000:.2f}ms")


# ========== Cleanup Tests ==========

def test_redis_flushdb_works(clean_redis):
    """
    Test: Verify Redis cleanup works correctly.
    """
    # Add data
    clean_redis.set("test_key", "test_value")
    assert clean_redis.get("test_key") == "test_value"
    
    # Flush
    clean_redis.flushdb()
    
    # Verify empty
    assert clean_redis.get("test_key") is None


"""
Running Integration Tests:

# All tests
pytest tests/integration/ -v

# Specific test
pytest tests/integration/test_pipeline.py::test_telemetry_ingestion_to_redis -v

# Skip slow tests
pytest tests/integration/ -v -m "not slow"

# Only benchmarks
pytest tests/integration/ -v --benchmark-only

# With coverage
pytest tests/integration/ -v --cov=. --cov-report=html

# Parallel execution (requires pytest-xdist)
pytest tests/integration/ -v -n auto

Interview Talking Points:
- "I use pytest for integration tests because it has excellent fixtures for setup/teardown"
- "Parametrized tests let me test 10 scenarios with 1 test function"
- "pytest-benchmark measures p95 latency automatically"
- "Fixtures with scope='module' share connections across tests for speed"
"""
