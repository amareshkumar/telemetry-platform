#include <gtest/gtest.h>
#include "telemetry_processor/Task.h"

using namespace telemetry_processor;

TEST(TaskTest, CreateTask) {
    auto task = Task::create(
        "compute",
        R"({"operation": "test"})",
        Priority::HIGH,
        3
    );
    
    EXPECT_FALSE(task.id.empty());
    EXPECT_EQ(task.type, "compute");
    EXPECT_EQ(task.priority, Priority::HIGH);
    EXPECT_EQ(task.status, TaskStatus::PENDING);
    EXPECT_EQ(task.retry_count, 0);
    EXPECT_EQ(task.max_retries, 3);
}

TEST(TaskTest, JSONSerialization) {
    auto task = Task::create("test", R"({"data": 123})", Priority::NORMAL, 5);
    
    // Serialize
    auto json = task.to_json();
    EXPECT_FALSE(json["id"].is_null());
    EXPECT_EQ(json["type"], "test");
    EXPECT_EQ(json["priority"], static_cast<int>(Priority::NORMAL));
    
    // Deserialize
    auto task2 = Task::from_json(json);
    EXPECT_EQ(task.id, task2.id);
    EXPECT_EQ(task.type, task2.type);
    EXPECT_EQ(task.priority, task2.priority);
    EXPECT_EQ(task.max_retries, task2.max_retries);
}

TEST(TaskTest, UUIDGeneration) {
    auto uuid1 = generate_uuid();
    auto uuid2 = generate_uuid();
    
    EXPECT_FALSE(uuid1.empty());
    EXPECT_FALSE(uuid2.empty());
    EXPECT_NE(uuid1, uuid2);  // Should be unique
    
    // UUID format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    EXPECT_EQ(uuid1.length(), 36);
    EXPECT_EQ(uuid1[8], '-');
    EXPECT_EQ(uuid1[13], '-');
    EXPECT_EQ(uuid1[18], '-');
    EXPECT_EQ(uuid1[23], '-');
}

TEST(TaskTest, PriorityToString) {
    EXPECT_EQ(priority_to_string(Priority::HIGH), "HIGH");
    EXPECT_EQ(priority_to_string(Priority::NORMAL), "NORMAL");
    EXPECT_EQ(priority_to_string(Priority::LOW), "LOW");
}

TEST(TaskTest, StatusToString) {
    EXPECT_EQ(status_to_string(TaskStatus::PENDING), "PENDING");
    EXPECT_EQ(status_to_string(TaskStatus::RUNNING), "RUNNING");
    EXPECT_EQ(status_to_string(TaskStatus::COMPLETED), "COMPLETED");
    EXPECT_EQ(status_to_string(TaskStatus::FAILED), "FAILED");
    EXPECT_EQ(status_to_string(TaskStatus::CANCELLED), "CANCELLED");
}

TEST(TaskTest, RoundTripSerialization) {
    // Create task with all fields set
    auto original = Task::create("io", R"({"file": "data.txt"})", Priority::LOW, 2);
    original.status = TaskStatus::RUNNING;
    original.retry_count = 1;
    original.worker_id = "worker-001";
    
    // Serialize and deserialize
    auto json = original.to_json();
    auto restored = Task::from_json(json);
    
    // Verify all fields match
    EXPECT_EQ(original.id, restored.id);
    EXPECT_EQ(original.type, restored.type);
    EXPECT_EQ(original.payload, restored.payload);
    EXPECT_EQ(original.priority, restored.priority);
    EXPECT_EQ(original.status, restored.status);
    EXPECT_EQ(original.retry_count, restored.retry_count);
    EXPECT_EQ(original.max_retries, restored.max_retries);
    EXPECT_EQ(original.worker_id, restored.worker_id);
}
