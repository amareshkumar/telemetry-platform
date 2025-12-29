/**
 * High Concurrency Test - Tests gateway with improved threading
 * Validates 100+ concurrent connections work without timeouts
 */
import http from 'k6/http';
import { check, sleep } from 'k6';
import { Counter, Trend, Rate } from 'k6/metrics';

const telemetryCounter = new Counter('telemetry_requests_total');
const telemetryDuration = new Trend('telemetry_request_duration_ms');
const telemetryErrors = new Rate('telemetry_error_rate');

export let options = {
    stages: [
        { duration: '10s', target: 50 },   // Warm up
        { duration: '20s', target: 100 },  // Ramp to 100
        { duration: '30s', target: 100 },  // Hold 100
        { duration: '10s', target: 200 },  // Spike test
        { duration: '10s', target: 100 },  // Back to 100
        { duration: '10s', target: 0 },    // Ramp down
    ],
    thresholds: {
        'http_req_duration': ['p(95)<200', 'p(99)<500'],
        'telemetry_error_rate': ['rate<0.01'],  // Less than 1% errors
        'http_req_failed': ['rate<0.01'],
    },
};

const BASE_URL = 'http://localhost:8080';

export function setup() {
    console.log('===========================================');
    console.log('High Concurrency Test - Gateway Threading');
    console.log('===========================================');
    console.log(`Target: ${BASE_URL}`);
    console.log('Testing: 100-200 concurrent connections');
    console.log('Expected: <1% error rate, p95 < 200ms');
    console.log('===========================================\n');
    
    const healthResponse = http.get(`${BASE_URL}/health`);
    if (healthResponse.status !== 200) {
        throw new Error(`Gateway not healthy: ${healthResponse.status}`);
    }
    console.log('✓ Gateway health check passed\n');
}

export default function () {
    const payload = {
        device_id: `sensor-${__VU}`,
        type: 'temperature',
        value: 20.0 + Math.random() * 15,
        unit: 'celsius',
        priority: Math.random() < 0.1 ? 'HIGH' : 'MEDIUM',
        timestamp: new Date().toISOString(),
    };
    
    const startTime = Date.now();
    const response = http.post(
        `${BASE_URL}/telemetry`,
        JSON.stringify(payload),
        {
            headers: { 'Content-Type': 'application/json' },
            timeout: '10s',  // 10 second timeout
        }
    );
    const duration = Date.now() - startTime;
    
    const success = check(response, {
        'status is 200': (r) => r.status === 200,
        'response time < 200ms': (r) => r.timings.duration < 200,
        'response has ok': (r) => {
            try {
                return JSON.parse(r.body).ok === true;
            } catch {
                return false;
            }
        },
    });
    
    telemetryCounter.add(1);
    telemetryDuration.add(duration);
    telemetryErrors.add(!success);
    
    sleep(0.5); // 500ms between requests per VU
}

export function teardown(data) {
    console.log('\n===========================================');
    console.log('Test Complete');
    console.log('===========================================');
    console.log('Check thresholds above for pass/fail status');
    console.log('Target: <1% error rate, p95 < 200ms');
}
