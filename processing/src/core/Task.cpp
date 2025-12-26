#include "telemetry_processor/Task.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace telemetry_processor {

std::string generate_uuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    
    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4"; // Version 4
    
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    
    ss << dis2(gen); // Variant
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }
    
    return ss.str();
}

std::string priority_to_string(Priority p) {
    switch (p) {
        case Priority::HIGH: return "HIGH";
        case Priority::NORMAL: return "NORMAL";
        case Priority::LOW: return "LOW";
        default: return "UNKNOWN";
    }
}

std::string status_to_string(TaskStatus s) {
    switch (s) {
        case TaskStatus::PENDING: return "PENDING";
        case TaskStatus::RUNNING: return "RUNNING";
        case TaskStatus::COMPLETED: return "COMPLETED";
        case TaskStatus::FAILED: return "FAILED";
        case TaskStatus::CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}

nlohmann::json Task::to_json() const {
    return nlohmann::json{
        {"id", id},
        {"type", type},
        {"payload", payload},
        {"priority", static_cast<int>(priority)},
        {"status", static_cast<int>(status)},
        {"retry_count", retry_count},
        {"max_retries", max_retries},
        {"created_at", std::chrono::system_clock::to_time_t(created_at)},
        {"updated_at", std::chrono::system_clock::to_time_t(updated_at)},
        {"worker_id", worker_id}
    };
}

Task Task::from_json(const nlohmann::json& j) {
    Task task;
    task.id = j.value("id", "");
    task.type = j.value("type", "");
    task.payload = j.value("payload", "");
    task.priority = static_cast<Priority>(j.value("priority", 1));
    task.status = static_cast<TaskStatus>(j.value("status", 0));
    task.retry_count = j.value("retry_count", 0);
    task.max_retries = j.value("max_retries", 3);
    
    auto created_timestamp = j.value("created_at", 0);
    task.created_at = std::chrono::system_clock::from_time_t(created_timestamp);
    
    auto updated_timestamp = j.value("updated_at", 0);
    task.updated_at = std::chrono::system_clock::from_time_t(updated_timestamp);
    
    task.worker_id = j.value("worker_id", "");
    
    return task;
}

Task Task::create(
    const std::string& type,
    const std::string& payload,
    Priority priority,
    int max_retries
) {
    Task task;
    task.id = generate_uuid();
    task.type = type;
    task.payload = payload;
    task.priority = priority;
    task.status = TaskStatus::PENDING;
    task.retry_count = 0;
    task.max_retries = max_retries;
    task.created_at = std::chrono::system_clock::now();
    task.updated_at = task.created_at;
    task.worker_id = "";
    
    return task;
}

} // namespace telemetry_processor
