#include "telemetry_processor/Task.h"
#include "telemetry_processor/RedisClient.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "DistQueue Simple Producer Example\n";
    std::cout << "==================================\n\n";
    
    // Connect to Redis
    telemetry_processor::RedisClient redis;
    if (!redis.connect()) {
        std::cerr << "Failed to connect to Redis\n";
        return 1;
    }
    std::cout << "Connected to Redis: " << redis.ping() << "\n\n";
    
    // Queue for pending tasks
    const std::string queue_key = "distqueue:tasks:pending";
    
    // Create and submit tasks
    std::cout << "Submitting tasks...\n";
    for (int i = 1; i <= 5; ++i) {
        // Create task payload
        nlohmann::json payload = {
            {"task_number", i},
            {"operation", "process_data"},
            {"data", "sample_" + std::to_string(i)}
        };
        
        // Create task
        auto priority = (i == 1) ? telemetry_processor::Priority::HIGH : telemetry_processor::Priority::NORMAL;
        auto task = telemetry_processor::Task::create(
            "compute",
            payload.dump(),
            priority,
            3  // max retries
        );
        
        // Serialize and push to queue
        auto task_json = task.to_json();
        redis.rpush(queue_key, task_json.dump());
        
        std::cout << "  [" << i << "] Task " << task.id.substr(0, 8) << "... "
                  << "Priority: " << telemetry_processor::priority_to_string(priority) << "\n";
        
        // Small delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Check queue status
    auto queue_length = redis.llen(queue_key);
    std::cout << "\nTasks in queue: " << queue_length << "\n";
    
    std::cout << "\n✓ Tasks submitted successfully!\n";
    std::cout << "Run workers to process these tasks.\n";
    
    return 0;
}
