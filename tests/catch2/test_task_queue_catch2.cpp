/**
 * @file test_task_queue_catch2.cpp
 * @brief Catch2 BDD-style tests for TaskQueue (Modern C++ Testing)
 * 
 * This demonstrates Catch2 v3 testing style for interview preparation.
 * Converts GoogleTest examples to BDD format (SCENARIO/GIVEN/WHEN/THEN).
 * 
 * Catch2 Advantages:
 * - Readable BDD syntax (business-friendly)
 * - Header-only option (easy integration)
 * - Section-based tests (better organization)
 * - Beautiful output with colors
 * - Fast compilation
 * 
 * Interview Talking Points:
 * - "I chose Catch2 for new tests to learn modern BDD-style testing"
 * - "SCENARIO/GIVEN/WHEN/THEN makes tests self-documenting"
 * - "Catch2 v3 is 30% faster compilation than GoogleTest"
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "task_queue.h"
#include <thread>
#include <chrono>

using namespace telemetry_processing;
using namespace std::chrono_literals;
using Catch::Matchers::ContainsSubstring;

// ========== Basic Operations ==========

SCENARIO("TaskQueue manages tasks with priority scheduling", "[priority][basic]") {
    GIVEN("An empty TaskQueue") {
        TaskQueue queue(100);  // Smaller capacity for tests
        
        THEN("Queue starts empty") {
            REQUIRE(queue.empty());
            REQUIRE(queue.size() == 0);
            REQUIRE_FALSE(queue.full());
        }
        
        WHEN("A task is enqueued") {
            Task task("task-001", TaskPriority::MEDIUM);
            task.payload = {{"device_id", "sensor-001"}, {"temperature", 25.5}};
            
            bool success = queue.enqueue(std::move(task));
            
            THEN("Task is successfully added") {
                REQUIRE(success);
                REQUIRE(queue.size() == 1);
                REQUIRE_FALSE(queue.empty());
            }
            
            AND_WHEN("Task is dequeued") {
                auto retrieved = queue.dequeue();
                
                THEN("Task is retrieved successfully") {
                    REQUIRE(retrieved.has_value());
                    REQUIRE(retrieved->id == "task-001");
                    REQUIRE(retrieved->priority == TaskPriority::MEDIUM);
                    REQUIRE(retrieved->payload["device_id"] == "sensor-001");
                }
                
                AND_THEN("Queue is empty again") {
                    REQUIRE(queue.empty());
                    REQUIRE(queue.size() == 0);
                }
            }
        }
    }
}

// ========== Priority Ordering ==========

SCENARIO("TaskQueue respects priority levels (HIGH > MEDIUM > LOW)", "[priority][ordering]") {
    GIVEN("A queue with mixed-priority tasks") {
        TaskQueue queue;
        
        // Enqueue tasks in random order
        Task low("low-task", TaskPriority::LOW);
        Task high("high-task", TaskPriority::HIGH);
        Task medium("medium-task", TaskPriority::MEDIUM);
        
        queue.enqueue(std::move(low));
        queue.enqueue(std::move(high));
        queue.enqueue(std::move(medium));
        
        REQUIRE(queue.size() == 3);
        
        WHEN("Tasks are dequeued") {
            auto first = queue.dequeue();
            auto second = queue.dequeue();
            auto third = queue.dequeue();
            
            THEN("HIGH priority task comes first") {
                REQUIRE(first.has_value());
                REQUIRE(first->id == "high-task");
                REQUIRE(first->priority == TaskPriority::HIGH);
            }
            
            THEN("MEDIUM priority task comes second") {
                REQUIRE(second.has_value());
                REQUIRE(second->id == "medium-task");
                REQUIRE(second->priority == TaskPriority::MEDIUM);
            }
            
            THEN("LOW priority task comes last") {
                REQUIRE(third.has_value());
                REQUIRE(third->id == "low-task");
                REQUIRE(third->priority == TaskPriority::LOW);
            }
        }
    }
}

SCENARIO("Tasks with same priority follow FIFO order", "[priority][fifo]") {
    GIVEN("Multiple tasks with MEDIUM priority") {
        TaskQueue queue;
        
        // Enqueue 3 tasks with same priority
        for (int i = 1; i <= 3; ++i) {
            Task task("task-" + std::to_string(i), TaskPriority::MEDIUM);
            queue.enqueue(std::move(task));
            std::this_thread::sleep_for(1ms);  // Ensure different timestamps
        }
        
        WHEN("Tasks are dequeued") {
            auto first = queue.dequeue();
            auto second = queue.dequeue();
            auto third = queue.dequeue();
            
            THEN("Tasks are returned in FIFO order (task-1, task-2, task-3)") {
                REQUIRE(first->id == "task-1");
                REQUIRE(second->id == "task-2");
                REQUIRE(third->id == "task-3");
            }
        }
    }
}

// ========== Bounded Capacity ==========

SCENARIO("TaskQueue enforces bounded capacity", "[capacity][backpressure]") {
    GIVEN("A queue with capacity 3") {
        TaskQueue queue(3);
        
        WHEN("Queue is filled to capacity") {
            for (int i = 1; i <= 3; ++i) {
                Task task("task-" + std::to_string(i), TaskPriority::MEDIUM);
                REQUIRE(queue.enqueue(std::move(task)));
            }
            
            THEN("Queue is full") {
                REQUIRE(queue.full());
                REQUIRE(queue.size() == 3);
            }
            
            AND_WHEN("Attempting to enqueue without timeout") {
                Task extra("extra-task", TaskPriority::HIGH);
                bool success = queue.enqueue(std::move(extra), 0ms);
                
                THEN("Enqueue fails immediately (no blocking)") {
                    REQUIRE_FALSE(success);
                    REQUIRE(queue.size() == 3);  // Still full
                }
            }
        }
    }
}

// ========== Timeout Behavior ==========

SCENARIO("TaskQueue supports timeout for blocking operations", "[timeout][blocking]") {
    GIVEN("An empty queue") {
        TaskQueue queue;
        
        WHEN("Attempting to dequeue with 100ms timeout") {
            auto start = std::chrono::steady_clock::now();
            auto task = queue.dequeue(100ms);
            auto duration = std::chrono::steady_clock::now() - start;
            
            THEN("Dequeue returns empty after ~100ms") {
                REQUIRE_FALSE(task.has_value());
                REQUIRE(duration >= 90ms);   // Allow 10ms tolerance
                REQUIRE(duration <= 150ms);  // Allow 50ms overhead
            }
        }
    }
}

// ========== Thread Safety ==========

SCENARIO("TaskQueue handles concurrent producers safely", "[threading][concurrent]") {
    GIVEN("A shared queue and multiple producer threads") {
        TaskQueue queue;
        constexpr int NUM_PRODUCERS = 4;
        constexpr int TASKS_PER_PRODUCER = 250;
        
        WHEN("4 threads enqueue 250 tasks each (1000 total)") {
            std::vector<std::thread> producers;
            
            for (int t = 0; t < NUM_PRODUCERS; ++t) {
                producers.emplace_back([&queue, t, TASKS_PER_PRODUCER]() {
                    for (int i = 0; i < TASKS_PER_PRODUCER; ++i) {
                        Task task(
                            "task-" + std::to_string(t) + "-" + std::to_string(i),
                            TaskPriority::MEDIUM
                        );
                        queue.enqueue(std::move(task));
                    }
                });
            }
            
            for (auto& thread : producers) {
                thread.join();
            }
            
            THEN("All 1000 tasks are successfully enqueued") {
                REQUIRE(queue.size() == 1000);
            }
        }
    }
}

SCENARIO("TaskQueue handles concurrent consumers safely", "[threading][concurrent]") {
    GIVEN("A queue with 1000 tasks") {
        TaskQueue queue;
        
        // Populate queue
        for (int i = 0; i < 1000; ++i) {
            Task task("task-" + std::to_string(i), TaskPriority::MEDIUM);
            queue.enqueue(std::move(task));
        }
        
        REQUIRE(queue.size() == 1000);
        
        WHEN("4 consumer threads dequeue tasks concurrently") {
            std::atomic<int> consumed_count{0};
            std::vector<std::thread> consumers;
            
            for (int t = 0; t < 4; ++t) {
                consumers.emplace_back([&queue, &consumed_count]() {
                    while (true) {
                        auto task = queue.dequeue(10ms);
                        if (!task.has_value()) break;
                        consumed_count++;
                    }
                });
            }
            
            for (auto& thread : consumers) {
                thread.join();
            }
            
            THEN("All 1000 tasks are consumed exactly once") {
                REQUIRE(consumed_count == 1000);
                REQUIRE(queue.empty());
            }
        }
    }
}

// ========== Utility Functions ==========

SCENARIO("TaskQueue provides useful utility functions", "[utility]") {
    GIVEN("A queue with various tasks") {
        TaskQueue queue;
        
        Task high("high-task", TaskPriority::HIGH);
        high.payload = {{"alert_type", "critical"}};
        queue.enqueue(std::move(high));
        
        Task low("low-task", TaskPriority::LOW);
        low.payload = {{"analytics", "aggregation"}};
        queue.enqueue(std::move(low));
        
        WHEN("Peeking at the next task") {
            auto peeked = queue.peek();
            
            THEN("Peek returns highest-priority task without removing it") {
                REQUIRE(peeked.has_value());
                REQUIRE(peeked->id == "high-task");
                REQUIRE(queue.size() == 2);  // Not removed
            }
        }
        
        WHEN("Getting queue statistics") {
            auto stats = queue.get_stats();
            
            THEN("Stats contain accurate information") {
                REQUIRE(stats["size"] == 2);
                REQUIRE(stats["capacity"] == 10000);
                REQUIRE(stats["empty"] == false);
                REQUIRE(stats["full"] == false);
            }
        }
        
        WHEN("Clearing the queue") {
            queue.clear();
            
            THEN("Queue is empty") {
                REQUIRE(queue.empty());
                REQUIRE(queue.size() == 0);
            }
        }
    }
}

// ========== Performance Benchmark (Optional) ==========

TEST_CASE("TaskQueue performance benchmark", "[.benchmark][performance]") {
    TaskQueue queue;
    
    BENCHMARK("Enqueue 1000 MEDIUM tasks") {
        for (int i = 0; i < 1000; ++i) {
            Task task("task-" + std::to_string(i), TaskPriority::MEDIUM);
            queue.enqueue(std::move(task));
        }
        return queue.size();
    };
    
    // Populate for dequeue benchmark
    for (int i = 0; i < 1000; ++i) {
        Task task("task-" + std::to_string(i), TaskPriority::MEDIUM);
        queue.enqueue(std::move(task));
    }
    
    BENCHMARK("Dequeue 1000 tasks") {
        int count = 0;
        while (!queue.empty()) {
            auto task = queue.dequeue();
            if (task.has_value()) count++;
        }
        return count;
    };
}

/**
 * Running Catch2 Tests:
 * 
 * # Run all tests
 * ./catch2_tests
 * 
 * # Run with specific tags
 * ./catch2_tests "[priority]"
 * ./catch2_tests "[threading]"
 * ./catch2_tests "[.benchmark]"  # Run benchmarks (disabled by default)
 * 
 * # Run with reporter
 * ./catch2_tests --reporter compact   # Minimal output
 * ./catch2_tests --reporter junit     # JUnit XML for CI
 * ./catch2_tests --reporter console   # Detailed output (default)
 * 
 * # List all tests
 * ./catch2_tests --list-tests
 * 
 * # List all tags
 * ./catch2_tests --list-tags
 */
