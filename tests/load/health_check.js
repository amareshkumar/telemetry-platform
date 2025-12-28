/**
 * k6 Health Check Load Test
 * 
 * Simple smoke test for gateway availability.
 * Run this before full load tests to ensure gateway is responsive.
 * 
 * Run:
 * k6 run tests/load/health_check.js --vus 10 --duration 30s
 */

import http from 'k6/http';
import { check, sleep } from 'k6';

export let options = {
    vus: 10,  // 10 virtual users
    duration: '30s',
    
    thresholds: {
        'http_req_duration': ['p(95)<50'],  // Health check should be < 50ms
        'http_req_failed': ['rate<0.001'],  // < 0.1% error rate
    },
};

const BASE_URL = __ENV.BASE_URL || 'http://localhost:8080';

export default function () {
    const response = http.get(`${BASE_URL}/health`);
    
    check(response, {
        'status is 200': (r) => r.status === 200,
        'response time < 50ms': (r) => r.timings.duration < 50,
        'has status field': (r) => JSON.parse(r.body).status === 'healthy',
    });
    
    sleep(1);
}

export function setup() {
    console.log(`Health check smoke test: ${BASE_URL}`);
}
