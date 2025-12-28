#include <gtest/gtest.h>
#include "task_queue.h"
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace telemetry_processing;
using namespace std::chrono_literals;

/**
 * @file test_task_queue.cpp
 * @brief Comprehensive tests for TaskQueue priority scheduling
 * 
 * Test Coverage:
 * - Basic operations (enqueue, dequeue, peek)
 * - Priority ordering (HIGH > MEDIUM > LOW)
 * - FIFO within same priority
 * - Thread safety (concurrent producers/consumers)
 * - Bounded capacity (backpressure)
 * - Timeout behavior (blocking operations)
 * - Edge cases (empty, full, shutdown)
 */

// ========== Basic Operations Tests ==========

TEST(TaskQueueTest, DefaultConstructor) {
    TaskQueue queue;
    EXPECT_EQ(queue.size(), 0);
    EXPECT_TRUE(queue.empty());
    EXPECT_FALSE(queue.full());
    EXPECT_EQ(queue.capacity(), 10000);  // Default capacity
}

TEST(TaskQueueTest, CustomCapacity) {
    TaskQueue queue(100);
    EXPECT_EQ(queue.capacity(), 100);
    EXPECT_TRUE(queue.empty());
}

TEST(TaskQueueTest, EnqueueDequeue) {
    TaskQueue queue;
    
    Task task("task-001", TaskPriority::MEDIUM);
    task.payload = {{"device_id", "sensor-001"}};
    
    ASSERT_TRUE(queue.enqueue(std::move(task)));
    EXPECT_EQ(queue.size(), 1);
    EXPECT_FALSE(queue.empty());
    
    auto dequeued = queue.dequeue();
    ASSERT_TRUE(dequeued.has_value());
    EXPECT_EQ(dequeued->id, "task-001");
    EXPECT_EQ(dequeued->priority, TaskPriority::MEDIUM);
    EXPECT_EQ(dequeued->payload["device_id"], "sensor-001");
    
    EXPECT_TRUE(queue.empty());
}

TEST(TaskQueueTest, PeekWithoutRemoval) {
    TaskQueue queue;
    
    Task task("task-001", TaskPriority::HIGH);
    queue.enqueue(std::move(task));
    
    auto peeked = queue.peek();
    ASSERT_TRUE(peeked.has_value());
    EXPECT_EQ(peeked->id, "task-001");
    
    // Queue should still have the task
    EXPECT_EQ(queue.size(), 1);
    
    // Dequeue should return same task
    auto dequeued = queue.dequeue();
    ASSERT_TRUE(dequeued.has_value());
    EXPECT_EQ(dequeued->id, "task-001");
}

TEST(TaskQueueTest, DequeueEmptyQueue) {
    TaskQueue queue;
    
    auto task = queue.dequeue();
    EXPECT_FALSE(task.has_value());
}

TEST(TaskQueueTest, PeekEmptyQueue) {
    TaskQueue queue;
    
    auto task = queue.peek();
    EXPECT_FALSE(task.has_value());
}

// ========== Priority Ordering Tests ==========

TEST(TaskQueueTest, PriorityOrdering) {
    TaskQueue queue;
    
    // Enqueue in random order
    queue.enqueue(Task("low-1", TaskPriority::LOW));
    queue.enqueue(Task("high-1", TaskPriority::HIGH));
    queue.enqueue(Task("medium-1", TaskPriority::MEDIUM));
    queue.enqueue(Task("high-2", TaskPriority::HIGH));
    queue.enqueue(Task("low-2", TaskPriority::LOW));
    
    EXPECT_EQ(queue.size(), 5);
    
    // Dequeue should return HIGH priority first
    auto task1 = queue.dequeue();
    ASSERT_TRUE(task1.has_value());
    EXPECT_EQ(task1->priority, TaskPriority::HIGH);
    
    auto task2 = queue.dequeue();
    ASSERT_TRUE(task2.has_value());
    EXPECT_EQ(task2->priority, TaskPriority::HIGH);
    
    // Then MEDIUM
    auto task3 = queue.dequeue();
    ASSERT_TRUE(task3.has_value());
    EXPECT_EQ(task3->priority, TaskPriority::MEDIUM);
    
    // Finally LOW
    auto task4 = queue.dequeue();
    ASSERT_TRUE(task4.has_value());
    EXPECT_EQ(task4->priority, TaskPriority::LOW);
    
    auto task5 = queue.dequeue();
    ASSERT_TRUE(task5.has_value());
    EXPECT_EQ(task5->priority, TaskPriority::LOW);
}

TEST(TaskQueueTest, FIFOWithinPriority) {
    TaskQueue queue;
    
    // Enqueue multiple tasks with same priority
    // Add small delays to ensure different timestamps
    queue.enqueue(Task("high-1", TaskPriority::HIGH));
    std::this_thread::sleep_for(1ms);
    
    queue.enqueue(Task("high-2", TaskPriority::HIGH));
    std::this_thread::sleep_for(1ms);
    
    queue.enqueue(Task("high-3", TaskPriority::HIGH));
    
    // Should dequeue in FIFO order (earliest first)
    auto task1 = queue.dequeue();
    ASSERT_TRUE(task1.has_value());
    EXPECT_EQ(task1->id, "high-1");
    
    auto task2 = queue.dequeue();
    ASSERT_TRUE(task2.has_value());
    EXPECT_EQ(task2->id, "high-2");
    
    auto task3 = queue.dequeue();
    ASSERT_TRUE(task3.has_value());
    EXPECT_EQ(task3->id, "high-3");
}

// ========== Bounded Capacity Tests ==========

TEST(TaskQueueTest, EnqueueFullQueue) {
    TaskQueue queue(3);  // Capacity 3
    
    ASSERT_TRUE(queue.enqueue(Task("task-1", TaskPriority::MEDIUM)));
    ASSERT_TRUE(queue.enqueue(Task("task-2", TaskPriority::MEDIUM)));
    ASSERT_TRUE(queue.enqueue(Task("task-3", TaskPriority::MEDIUM)));
    
    EXPECT_TRUE(queue.full());
    EXPECT_EQ(queue.size(), 3);
    
    // Non-blocking enqueue should fail
    EXPECT_FALSE(queue.enqueue(Task("task-4", TaskPriority::MEDIUM)));
    EXPECT_EQ(queue.size(), 3);  // Size unchanged
}

TEST(TaskQueueTest, EnqueueWithTimeoutSuccess) {
    TaskQueue queue(2);
    
    queue.enqueue(Task("task-1", TaskPriority::MEDIUM));
    queue.enqueue(Task("task-2", TaskPriority::MEDIUM));
    EXPECT_TRUE(queue.full());
    
    // Start consumer thread to make space
    std::thread consumer([&queue]() {
        std::this_thread::sleep_for(100ms);
        queue.dequeue();  // Free one slot
    });
    
    // Enqueue with 500ms timeout (should succeed after 100ms)
    auto start = std::chrono::steady_clock::now();
    bool success = queue.enqueue(Task("task-3", TaskPriority::MEDIUM), 500ms);
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    EXPECT_TRUE(success);
    EXPECT_GE(elapsed, 100ms);  // Waited at least 100ms
    EXPECT_LT(elapsed, 500ms);  // But less than timeout
    
    consumer.join();
}

TEST(TaskQueueTest, EnqueueWithTimeoutFailure) {
    TaskQueue queue(2);
    
    queue.enqueue(Task("task-1", TaskPriority::MEDIUM));
    queue.enqueue(Task("task-2", TaskPriority::MEDIUM));
    EXPECT_TRUE(queue.full());
    
    // Enqueue with 100ms timeout (should fail - no consumer)
    auto start = std::chrono::steady_clock::now();
    bool success = queue.enqueue(Task("task-3", TaskPriority::MEDIUM), 100ms);
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    EXPECT_FALSE(success);
    EXPECT_GE(elapsed, 100ms);  // Waited full timeout
}

// ========== Timeout Tests ==========

TEST(TaskQueueTest, DequeueWithTimeoutSuccess) {
    TaskQueue queue;
    
    // Start producer thread to add task after delay
    std::thread producer([&queue]() {
        std::this_thread::sleep_for(100ms);
        queue.enqueue(Task("task-1", TaskPriority::HIGH));
    });
    
    // Dequeue with 500ms timeout (should succeed after 100ms)
    auto start = std::chrono::steady_clock::now();
    auto task = queue.dequeue(500ms);
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    ASSERT_TRUE(task.has_value());
    EXPECT_EQ(task->id, "task-1");
    EXPECT_GE(elapsed, 100ms);  // Waited at least 100ms
    EXPECT_LT(elapsed, 500ms);  // But less than timeout
    
    producer.join();
}

TEST(TaskQueueTest, DequeueWithTimeoutFailure) {
    TaskQueue queue;
    
    // Dequeue with 100ms timeout (should fail - empty queue)
    auto start = std::chrono::steady_clock::now();
    auto task = queue.dequeue(100ms);
    auto elapsed = std::chrono::steady_clock::now() - start;
    
    EXPECT_FALSE(task.has_value());
    EXPECT_GE(elapsed, 100ms);  // Waited full timeout
}

// ========== Thread Safety Tests ==========

TEST(TaskQueueTest, ConcurrentProducers) {
    TaskQueue queue(1000);
    const int num_producers = 4;
    const int tasks_per_producer = 250;
    
    std::vector<std::thread> producers;
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&queue, i, tasks_per_producer]() {
            for (int j = 0; j < tasks_per_producer; ++j) {
                std::string id = "task-" + std::to_string(i) + "-" + std::to_string(j);
                Task task(id, TaskPriority::MEDIUM);
                queue.enqueue(std::move(task));
            }
        });
    }
    
    for (auto& t : producers) {
        t.join();
    }
    
    // All tasks should be enqueued
    EXPECT_EQ(queue.size(), num_producers * tasks_per_producer);
}

TEST(TaskQueueTest, ConcurrentConsumers) {
    TaskQueue queue;
    const int num_tasks = 1000;
    
    // Enqueue tasks
    for (int i = 0; i < num_tasks; ++i) {
        queue.enqueue(Task("task-" + std::to_string(i), TaskPriority::MEDIUM));
    }
    
    // Consume with multiple threads
    std::atomic<int> tasks_consumed{0};
    const int num_consumers = 4;
    
    std::vector<std::thread> consumers;
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&queue, &tasks_consumed]() {
            while (true) {
                auto task = queue.dequeue();
                if (!task) break;  // Queue empty
                tasks_consumed++;
            }
        });
    }
    
    for (auto& t : consumers) {
        t.join();
    }
    
    // All tasks should be consumed
    EXPECT_EQ(tasks_consumed, num_tasks);
    EXPECT_TRUE(queue.empty());
}

TEST(TaskQueueTest, ProducerConsumerPattern) {
    TaskQueue queue(100);
    const int num_tasks = 500;
    std::atomic<int> tasks_consumed{0};
    std::atomic<bool> producer_done{false};
    
    // Producer thread
    std::thread producer([&queue, num_tasks, &producer_done]() {
        for (int i = 0; i < num_tasks; ++i) {
            Task task("task-" + std::to_string(i), TaskPriority::MEDIUM);
            // Blocking enqueue with timeout
            while (!queue.enqueue(std::move(task), 100ms)) {
                // Retry if queue full
            }
        }
        producer_done = true;
    });
    
    // Consumer thread
    std::thread consumer([&queue, &tasks_consumed, &producer_done]() {
        while (!producer_done || !queue.empty()) {
            auto task = queue.dequeue(50ms);
            if (task) {
                tasks_consumed++;
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_EQ(tasks_consumed, num_tasks);
    EXPECT_TRUE(queue.empty());
}

// ========== Clear Test ==========

TEST(TaskQueueTest, ClearQueue) {
    TaskQueue queue;
    
    queue.enqueue(Task("task-1", TaskPriority::HIGH));
    queue.enqueue(Task("task-2", TaskPriority::MEDIUM));
    queue.enqueue(Task("task-3", TaskPriority::LOW));
    
    EXPECT_EQ(queue.size(), 3);
    
    queue.clear();
    
    EXPECT_EQ(queue.size(), 0);
    EXPECT_TRUE(queue.empty());
}

// ========== Statistics Test ==========

TEST(TaskQueueTest, GetStats) {
    TaskQueue queue(100);
    
    queue.enqueue(Task("task-1", TaskPriority::HIGH));
    queue.enqueue(Task("task-2", TaskPriority::MEDIUM));
    queue.enqueue(Task("task-3", TaskPriority::LOW));
    
    auto stats = queue.get_stats();
    
    EXPECT_EQ(stats["current_size"], 3);
    EXPECT_EQ(stats["capacity"], 100);
    EXPECT_DOUBLE_EQ(stats["utilization"], 3.0);  // 3% full
}

// ========== Utility Tests ==========

TEST(TaskQueueTest, TaskPriorityToString) {
    EXPECT_EQ(to_string(TaskPriority::HIGH), "HIGH");
    EXPECT_EQ(to_string(TaskPriority::MEDIUM), "MEDIUM");
    EXPECT_EQ(to_string(TaskPriority::LOW), "LOW");
}

TEST(TaskQueueTest, TaskConstructors) {
    // Default constructor
    Task task1;
    EXPECT_EQ(task1.priority, TaskPriority::MEDIUM);
    EXPECT_TRUE(task1.payload.empty());
    
    // Constructor with ID and priority
    Task task2("task-001", TaskPriority::HIGH);
    EXPECT_EQ(task2.id, "task-001");
    EXPECT_EQ(task2.priority, TaskPriority::HIGH);
    
    // Full constructor with payload
    nlohmann::json payload = {{"device_id", "sensor-001"}, {"temperature", 25.5}};
    Task task3("task-002", TaskPriority::LOW, payload);
    EXPECT_EQ(task3.id, "task-002");
    EXPECT_EQ(task3.priority, TaskPriority::LOW);
    EXPECT_EQ(task3.payload["device_id"], "sensor-001");
    EXPECT_DOUBLE_EQ(task3.payload["temperature"], 25.5);
}

// ========== Performance Benchmark (Optional) ==========

TEST(TaskQueueTest, DISABLED_PerformanceBenchmark) {
    TaskQueue queue(10000);
    const int num_tasks = 10000;
    
    // Benchmark enqueue
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_tasks; ++i) {
        Task task("task-" + std::to_string(i), TaskPriority::MEDIUM);
        queue.enqueue(std::move(task));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto enqueue_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Enqueue: " << num_tasks << " tasks in " 
              << enqueue_duration.count() << " us" << std::endl;
    std::cout << "Enqueue rate: " 
              << (num_tasks * 1000000.0 / enqueue_duration.count()) << " ops/sec" << std::endl;
    
    // Benchmark dequeue
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_tasks; ++i) {
        queue.dequeue();
    }
    end = std::chrono::high_resolution_clock::now();
    auto dequeue_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Dequeue: " << num_tasks << " tasks in " 
              << dequeue_duration.count() << " us" << std::endl;
    std::cout << "Dequeue rate: " 
              << (num_tasks * 1000000.0 / dequeue_duration.count()) << " ops/sec" << std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
