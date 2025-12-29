/**
 * k6 test with Prometheus output for Grafana
 * Exports metrics that Grafana can scrape
 */
import http from 'k6/http';
import { check, sleep } from 'k6';
import { Counter, Trend, Rate } from 'k6/metrics';

// Custom metrics
const telemetryCounter = new Counter('telemetry_requests_total');
const telemetryDuration = new Trend('telemetry_request_duration_ms');
const telemetryErrors = new Rate('telemetry_error_rate');

export let options = {
    stages: [
        { duration: '30s', target: 50 },
        { duration: '1m', target: 100 },
        { duration: '30s', target: 0 },
    ],
    thresholds: {
        'http_req_duration': ['p(95)<200', 'p(99)<500'],
        'telemetry_error_rate': ['rate<0.01'],
    },
};

const BASE_URL = 'http://localhost:8080';

export function setup() {
    const healthResponse = http.get(`${BASE_URL}/health`);
    if (healthResponse.status !== 200) {
        throw new Error(`Gateway not healthy: ${healthResponse.status}`);
    }
    console.log('✓ Gateway health check passed');
}

export default function () {
    const payload = {
        device_id: `sensor-${__VU}`,
        type: 'temperature',
        value: 20.0 + Math.random() * 15,
        unit: 'celsius',
        priority: Math.random() < 0.1 ? 'HIGH' : 'MEDIUM',
        timestamp: new Date().toISOString(),
        metadata: {
            location: `zone-${Math.floor(__VU / 10)}`,
            firmware_version: '2.3.1',
        },
    };
    
    const startTime = Date.now();
    const response = http.post(
        `${BASE_URL}/telemetry`,
        JSON.stringify(payload),
        {
            headers: { 'Content-Type': 'application/json' },
        }
    );
    const duration = Date.now() - startTime;
    
    const success = check(response, {
        'status is 200': (r) => r.status === 200,
        'response has ok': (r) => {
            try {
                return JSON.parse(r.body).ok === true;
            } catch {
                return false;
            }
        },
    });
    
    // Record metrics
    telemetryCounter.add(1);
    telemetryDuration.add(duration);
    telemetryErrors.add(!success);
    
    sleep(Math.random() * 2 + 1); // 1-3 seconds between requests
}

export function teardown(data) {
    console.log('\n✓ Test completed successfully');
}
