#include "telemetry_processor/Task.h"
#include "telemetry_processor/RedisClient.h"
#include <iostream>

int main() {
    std::cout << "TelemetryTaskProcessor - Day 1 Demo\n";
    std::cout << "======================\n\n";
    
    // Test 1: Task creation and serialization
    std::cout << "Test 1: Task Creation\n";
    auto task = telemetry_processor::Task::create(
        "compute",
        R"({"operation": "matrix_multiply", "size": 1000})",
        telemetry_processor::Priority::HIGH,
        3
    );
    
    std::cout << "  Task ID: " << task.id << "\n";
    std::cout << "  Type: " << task.type << "\n";
    std::cout << "  Priority: " << telemetry_processor::priority_to_string(task.priority) << "\n";
    std::cout << "  Status: " << telemetry_processor::status_to_string(task.status) << "\n";
    
    // Test 2: JSON serialization
    std::cout << "\nTest 2: JSON Serialization\n";
    auto json = task.to_json();
    std::cout << "  JSON: " << json.dump(2) << "\n";
    
    // Test 3: JSON deserialization
    std::cout << "\nTest 3: JSON Deserialization\n";
    auto task2 = telemetry_processor::Task::from_json(json);
    std::cout << "  Deserialized ID: " << task2.id << "\n";
    std::cout << "  Match: " << (task.id == task2.id ? "✓" : "✗") << "\n";
    
    // Test 4: Redis client
    std::cout << "\nTest 4: Redis Client (Mock)\n";
    telemetry_processor::RedisClient redis;
    
    if (redis.connect()) {
        std::cout << "  Connected: ✓\n";
        
        auto pong = redis.ping();
        std::cout << "  Ping: " << pong << "\n";
        
        // Test RPUSH/BLPOP
        std::string queue_key = "distqueue:tasks:pending";
        redis.rpush(queue_key, json.dump());
        std::cout << "  Pushed task to queue: ✓\n";
        
        auto queue_len = redis.llen(queue_key);
        std::cout << "  Queue length: " << queue_len << "\n";
        
        auto popped = redis.blpop(queue_key);
        if (popped) {
            std::cout << "  Popped task: ✓\n";
            auto task3 = telemetry_processor::Task::from_json(nlohmann::json::parse(*popped));
            std::cout << "  Popped Task ID: " << task3.id << "\n";
        }
        
        // Test SET/GET
        std::string key = "distqueue:task:" + task.id;
        redis.set(key, json.dump());
        auto value = redis.get(key);
        std::cout << "  SET/GET: " << (value.has_value() ? "✓" : "✗") << "\n";
        
    } else {
        std::cout << "  Connection failed: ✗\n";
    }
    
    std::cout << "\n======================\n";
    std::cout << "Day 1 Complete! ✓\n";
    std::cout << "\nNext Steps:\n";
    std::cout << "  - Day 2: Producer API implementation\n";
    std::cout << "  - Day 3: Worker process\n";
    std::cout << "  - Day 4: Real Redis integration\n";
    
    return 0;
}
