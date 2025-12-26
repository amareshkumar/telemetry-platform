// Day 1: Redis Connection Test
// This test verifies basic Redis connectivity

#include "telemetry_common/redis_client.h"
#include <iostream>
#include <exception>

int main() {
    std::cout << "=== Day 1: Redis Connection Test ===" << std::endl;
    std::cout << std::endl;

    try {
        // Test 1: Connect to Redis
        std::cout << "[TEST 1] Connecting to Redis (localhost:6379)..." << std::endl;
        telemetry_common::RedisClient::ConnectionOptions opts;
        opts.host = "localhost";
        opts.port = 6379;
        
        telemetry_common::RedisClient client(opts);
        std::cout << "✅ Connected successfully!" << std::endl;
        std::cout << std::endl;

        // Test 2: PING command
        std::cout << "[TEST 2] Testing PING command..." << std::endl;
        if (client.ping()) {
            std::cout << "✅ PING successful - connection alive!" << std::endl;
        } else {
            std::cout << "❌ PING failed" << std::endl;
            return 1;
        }
        std::cout << std::endl;

        // Test 3: SET/GET operations
        std::cout << "[TEST 3] Testing SET/GET operations..." << std::endl;
        if (client.set("test:day1", "Hello from Day 1!")) {
            std::cout << "✅ SET successful" << std::endl;
        } else {
            std::cout << "❌ SET failed" << std::endl;
            return 1;
        }

        auto value = client.get("test:day1");
        if (value && *value == "Hello from Day 1!") {
            std::cout << "✅ GET successful: " << *value << std::endl;
        } else {
            std::cout << "❌ GET failed" << std::endl;
            return 1;
        }
        std::cout << std::endl;

        // Test 4: EXISTS/DEL operations
        std::cout << "[TEST 4] Testing EXISTS/DEL operations..." << std::endl;
        if (client.exists("test:day1")) {
            std::cout << "✅ EXISTS confirmed key exists" << std::endl;
        } else {
            std::cout << "❌ EXISTS failed" << std::endl;
            return 1;
        }

        if (client.del("test:day1") == 1) {
            std::cout << "✅ DEL successful" << std::endl;
        } else {
            std::cout << "❌ DEL failed" << std::endl;
            return 1;
        }

        if (!client.exists("test:day1")) {
            std::cout << "✅ EXISTS confirmed key deleted" << std::endl;
        } else {
            std::cout << "❌ Key still exists after delete" << std::endl;
            return 1;
        }
        std::cout << std::endl;

        // Test 5: LPUSH/RPOP (Task Queue simulation)
        std::cout << "[TEST 5] Testing LPUSH/RPOP (task queue)..." << std::endl;
        client.lpush("test:queue", "task1");
        client.lpush("test:queue", "task2");
        client.lpush("test:queue", "task3");
        std::cout << "✅ Pushed 3 tasks to queue" << std::endl;

        long long len = client.llen("test:queue");
        std::cout << "✅ Queue length: " << len << std::endl;

        auto task = client.rpop("test:queue");
        if (task && *task == "task1") {
            std::cout << "✅ Popped first task: " << *task << " (FIFO confirmed)" << std::endl;
        } else {
            std::cout << "❌ RPOP failed or wrong order" << std::endl;
            return 1;
        }

        // Cleanup
        client.del("test:queue");
        std::cout << std::endl;

        // Success!
        std::cout << "========================================" << std::endl;
        std::cout << "✅ Day 1 Complete: Redis connection working!" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;
        std::cout << "Interview Talking Points:" << std::endl;
        std::cout << "- redis-plus-plus: Modern C++ API with RAII" << std::endl;
        std::cout << "- Connection pooling: " << opts.pool_size << " connections" << std::endl;
        std::cout << "- Exception-safe operations with std::optional" << std::endl;
        std::cout << "- LPUSH/RPOP creates FIFO queue (O(1) operations)" << std::endl;
        std::cout << std::endl;

        return 0;
    }
    catch (const std::exception& e) {
        std::cout << "❌ Exception: " << e.what() << std::endl;
        std::cout << std::endl;
        std::cout << "NOTE: Make sure Redis is running!" << std::endl;
        std::cout << "  Docker: docker run -d -p 6379:6379 redis:7-alpine" << std::endl;
        std::cout << "  Windows: choco install redis-64" << std::endl;
        std::cout << "  Or use WSL: sudo service redis-server start" << std::endl;
        return 1;
    }
}
