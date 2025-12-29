/**
 * Simple k6 test - verify gateway works before complex load testing
 */
import http from 'k6/http';
import { check, sleep } from 'k6';

export let options = {
    vus: 10,
    duration: '30s',
};

const BASE_URL = 'http://localhost:8080';

export function setup() {
    console.log('Testing gateway health...');
    const healthResponse = http.get(`${BASE_URL}/health`);
    if (healthResponse.status !== 200) {
        throw new Error(`Gateway not healthy: ${healthResponse.status}`);
    }
    console.log('✓ Gateway is healthy\n');
}

export default function () {
    const payload = {
        device_id: `sensor-${__VU}`,
        type: 'temperature',
        value: 25.0 + Math.random() * 10,
        unit: 'celsius',
        priority: 'MEDIUM',
        timestamp: new Date().toISOString(),
    };
    
    const response = http.post(
        `${BASE_URL}/telemetry`,
        JSON.stringify(payload),
        {
            headers: { 'Content-Type': 'application/json' },
        }
    );
    
    check(response, {
        'status is 200': (r) => r.status === 200,
        'has ok field': (r) => JSON.parse(r.body).ok === true,
    });
    
    sleep(1);
}
