#pragma once

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace telemetry_processor {

/**
 * @brief Task priority levels
 * Higher priority tasks are processed first
 */
enum class Priority {
    HIGH = 0,
    NORMAL = 1,
    LOW = 2
};

/**
 * @brief Task execution status
 */
enum class TaskStatus {
    PENDING,      // Submitted but not started
    RUNNING,      // Currently being processed
    COMPLETED,    // Successfully finished
    FAILED,       // Failed after all retries
    CANCELLED     // Explicitly cancelled
};

/**
 * @brief Core Task data structure
 * 
 * Represents a unit of work to be executed by workers.
 * Tasks are serialized to JSON for storage in Redis.
 */
struct Task {
    std::string id;           // Unique identifier (UUID)
    std::string type;         // Task type: "compute", "io", "notify", etc.
    std::string payload;      // JSON serialized task data
    Priority priority;        // Execution priority
    TaskStatus status;        // Current status
    int retry_count;          // Number of times this task has been retried
    int max_retries;          // Maximum allowed retries
    std::chrono::system_clock::time_point created_at;   // Creation timestamp
    std::chrono::system_clock::time_point updated_at;   // Last update timestamp
    std::string worker_id;    // ID of worker processing this task (if running)
    
    /**
     * @brief Serialize task to JSON
     * @return JSON representation
     */
    nlohmann::json to_json() const;
    
    /**
     * @brief Deserialize task from JSON
     * @param j JSON object
     * @return Reconstructed Task
     */
    static Task from_json(const nlohmann::json& j);
    
    /**
     * @brief Create a new task with generated UUID
     * @param type Task type
     * @param payload Task data (JSON string)
     * @param priority Task priority
     * @param max_retries Maximum retry attempts
     * @return New Task instance
     */
    static Task create(
        const std::string& type,
        const std::string& payload,
        Priority priority = Priority::NORMAL,
        int max_retries = 3
    );
};

/**
 * @brief Generate a UUID v4 string
 * @return UUID string in format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 */
std::string generate_uuid();

/**
 * @brief Convert Priority to string
 */
std::string priority_to_string(Priority p);

/**
 * @brief Convert TaskStatus to string
 */
std::string status_to_string(TaskStatus s);

} // namespace telemetry_processor
