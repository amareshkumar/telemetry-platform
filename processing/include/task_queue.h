#pragma once

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>
#include <functional>
#include <nlohmann/json.hpp>

/**
 * @file task_queue.h
 * @brief Thread-safe priority-based task queue for telemetry processing
 * @author TelemetryHub Team
 * @date 2025-12-28
 * @version 0.3.0
 * 
 * @details
 * This file provides a production-ready priority queue implementation for
 * high-throughput telemetry processing with:
 * - **Priority Scheduling**: HIGH/MEDIUM/LOW priority levels
 * - **Thread Safety**: Full synchronization with mutex + condition variable
 * - **Bounded Capacity**: Configurable max size with backpressure
 * - **Blocking Operations**: enqueue() and dequeue() with timeout support
 * - **FIFO Within Priority**: Same-priority tasks processed in order
 * 
 * **Performance Characteristics**:
 * - enqueue(): O(log n) - binary heap insertion
 * - dequeue(): O(log n) - extract-min from heap
 * - peek(): O(1) - view highest priority task
 * - size()/empty(): O(1) - constant time
 * 
 * **Interview Talking Points**:
 * 1. **Priority Queue**: Why binary heap over sorted list? (O(log n) vs O(n))
 * 2. **Thread Safety**: Producer-consumer pattern with condition variables
 * 3. **Bounded Queue**: Backpressure mechanism (prevent memory exhaustion)
 * 4. **Timeout Handling**: std::chrono for type-safe time management
 * 5. **Modern C++17**: std::optional, lambda comparators, RAII lock guards
 * 
 * @see std::priority_queue for underlying data structure
 * @see std::condition_variable for blocking synchronization
 */

namespace telemetry_processing {

/**
 * @enum TaskPriority
 * @brief Priority levels for task scheduling
 * 
 * @details
 * Lower numeric values indicate higher priority (0 = highest).
 * This matches OS scheduling conventions (e.g., nice values in Unix).
 * 
 * **Priority Guidelines**:
 * - HIGH: Critical alerts, system health checks, SLA violations
 * - MEDIUM: Normal telemetry processing, analytics
 * - LOW: Batch jobs, data cleanup, non-urgent operations
 * 
 * @note Values are ordered for std::priority_queue (smallest = highest priority)
 */
enum class TaskPriority : int {
    HIGH = 0,     ///< Highest priority (processed first)
    MEDIUM = 1,   ///< Normal priority (default)
    LOW = 2       ///< Lowest priority (processed last)
};

/**
 * @brief Convert TaskPriority enum to human-readable string
 * @param priority Priority level
 * @return String representation ("HIGH", "MEDIUM", "LOW")
 */
inline std::string to_string(TaskPriority priority) {
    switch (priority) {
        case TaskPriority::HIGH: return "HIGH";
        case TaskPriority::MEDIUM: return "MEDIUM";
        case TaskPriority::LOW: return "LOW";
        default: return "UNKNOWN";
    }
}

/**
 * @struct Task
 * @brief Represents a single unit of work with priority and metadata
 * 
 * @details
 * A task encapsulates:
 * - **Priority**: For scheduling order
 * - **Timestamp**: For FIFO ordering within same priority
 * - **Payload**: Arbitrary JSON data for processing
 * - **ID**: Unique identifier for tracking
 * 
 * **Design Decisions**:
 * - JSON payload: Flexible schema-less data (vs rigid structs)
 * - Timestamp: std::chrono for precision (microseconds)
 * - ID: String UUID for distributed systems
 * 
 * @code
 * Task task;
 * task.id = "task-12345";
 * task.priority = TaskPriority::HIGH;
 * task.created_at = std::chrono::system_clock::now();
 * task.payload = {{"device_id", "sensor-001"}, {"temperature", 25.5}};
 * @endcode
 */
struct Task {
    std::string id;                                           ///< Unique task identifier
    TaskPriority priority = TaskPriority::MEDIUM;             ///< Scheduling priority
    std::chrono::system_clock::time_point created_at;         ///< Creation timestamp
    nlohmann::json payload;                                   ///< Task data (flexible JSON)
    
    /**
     * @brief Default constructor with current timestamp
     */
    Task() : created_at(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Constructor with ID and priority
     * @param task_id Unique task identifier
     * @param prio Task priority level
     */
    Task(std::string task_id, TaskPriority prio)
        : id(std::move(task_id))
        , priority(prio)
        , created_at(std::chrono::system_clock::now()) {}
    
    /**
     * @brief Full constructor with payload
     * @param task_id Unique task identifier
     * @param prio Task priority level
     * @param data Task payload (JSON)
     */
    Task(std::string task_id, TaskPriority prio, nlohmann::json data)
        : id(std::move(task_id))
        , priority(prio)
        , created_at(std::chrono::system_clock::now())
        , payload(std::move(data)) {}
};

/**
 * @class TaskQueue
 * @brief Thread-safe bounded priority queue for task scheduling
 * 
 * @details
 * Implements a producer-consumer pattern with priority scheduling.
 * Multiple producers can enqueue() tasks, multiple consumers can dequeue().
 * 
 * **Concurrency Model**:
 * - Mutex protects all shared state (queue, size counters)
 * - Condition variables for blocking operations:
 *   - `not_empty_`: Consumers wait when queue empty
 *   - `not_full_`: Producers wait when queue at capacity
 * - Lock guards ensure exception-safe unlocking
 * 
 * **Priority Scheduling**:
 * - Uses std::priority_queue with custom comparator
 * - Higher priority (lower enum value) processed first
 * - FIFO ordering within same priority (timestamp comparison)
 * 
 * **Bounded Capacity**:
 * - Prevents unbounded memory growth
 * - Provides backpressure to producers
 * - Configurable max size (default: 10,000 tasks)
 * 
 * **Interview Talking Points**:
 * 1. **Why bounded?** Prevent OOM in high-load scenarios
 * 2. **Why condition variables?** Efficient blocking (no busy-wait)
 * 3. **Why FIFO within priority?** Fairness + predictable latency
 * 4. **Thread safety patterns**: Lock guards, RAII, move semantics
 * 
 * **Usage Example**:
 * @code
 * // Create queue with capacity 1000
 * TaskQueue queue(1000);
 * 
 * // Producer thread
 * Task high_priority_task("alert-001", TaskPriority::HIGH);
 * high_priority_task.payload = {{"alert", "CPU overload"}};
 * 
 * if (queue.enqueue(std::move(high_priority_task), 5000ms)) {
 *     std::cout << "Task enqueued successfully" << std::endl;
 * } else {
 *     std::cerr << "Queue full, task dropped" << std::endl;
 * }
 * 
 * // Consumer thread
 * auto task = queue.dequeue(5000ms);
 * if (task) {
 *     processTask(*task);
 * } else {
 *     std::cout << "Queue empty, no task available" << std::endl;
 * }
 * @endcode
 * 
 * @warning Not copyable (queue contains unique state)
 * @note All methods are thread-safe
 * @see Task for task structure
 * @see TaskPriority for priority levels
 */
class TaskQueue {
public:
    /**
     * @brief Construct bounded task queue
     * @param max_capacity Maximum number of tasks (default: 10,000)
     * 
     * @details
     * Creates an empty queue with specified capacity.
     * Choose capacity based on:
     * - Expected throughput (events/sec)
     * - Processing latency (ms/task)
     * - Memory constraints (bytes/task)
     * 
     * **Capacity Guidelines**:
     * - Low throughput (< 100/sec): 1,000 tasks
     * - Medium throughput (< 1k/sec): 10,000 tasks
     * - High throughput (< 10k/sec): 100,000 tasks
     * 
     * @note Capacity 0 = unbounded (use with caution!)
     */
    explicit TaskQueue(size_t max_capacity = 10000);
    
    /**
     * @brief Destructor - automatically unblocks waiting threads
     * 
     * @details
     * Sets shutdown flag and notifies all waiting threads.
     * Prevents deadlock if queue destroyed while threads blocked.
     */
    ~TaskQueue();
    
    // Disable copy (queue contains unique state)
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;
    
    // Enable move (transfer ownership)
    TaskQueue(TaskQueue&&) noexcept = default;
    TaskQueue& operator=(TaskQueue&&) noexcept = default;
    
    /**
     * @brief Enqueue a task with optional timeout
     * 
     * @param task Task to enqueue (moved)
     * @param timeout Maximum wait time if queue full (default: 0 = no wait)
     * @return true if enqueued, false if timeout or shutdown
     * 
     * @details
     * If queue is full:
     * - timeout = 0: Returns false immediately
     * - timeout > 0: Waits up to timeout for space
     * - timeout = max: Waits indefinitely
     * 
     * **Time Complexity**: O(log n) where n = queue size
     * **Thread Safety**: Fully synchronized (safe from multiple threads)
     * 
     * @code
     * Task task("task-001", TaskPriority::HIGH);
     * 
     * // Non-blocking enqueue
     * if (!queue.enqueue(std::move(task))) {
     *     std::cerr << "Queue full, task dropped" << std::endl;
     * }
     * 
     * // Blocking enqueue with 5-second timeout
     * if (!queue.enqueue(std::move(task), 5000ms)) {
     *     std::cerr << "Queue full after 5 seconds" << std::endl;
     * }
     * @endcode
     * 
     * @warning Task is moved (do not use after enqueue)
     * @note Returns false if queue is shutting down
     */
    bool enqueue(Task task, 
                 std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    
    /**
     * @brief Dequeue highest-priority task with optional timeout
     * 
     * @param timeout Maximum wait time if queue empty (default: 0 = no wait)
     * @return Optional task (nullopt if timeout or shutdown)
     * 
     * @details
     * Returns highest-priority task (lowest enum value).
     * If queue is empty:
     * - timeout = 0: Returns nullopt immediately
     * - timeout > 0: Waits up to timeout for task
     * - timeout = max: Waits indefinitely
     * 
     * **Time Complexity**: O(log n) where n = queue size
     * **Thread Safety**: Fully synchronized
     * 
     * @code
     * // Non-blocking dequeue
     * auto task = queue.dequeue();
     * if (!task) {
     *     std::cout << "Queue empty" << std::endl;
     * }
     * 
     * // Blocking dequeue with 5-second timeout
     * auto task = queue.dequeue(5000ms);
     * if (task) {
     *     processTask(*task);
     * } else {
     *     std::cout << "No task after 5 seconds" << std::endl;
     * }
     * @endcode
     * 
     * @note Returns nullopt if queue is shutting down
     */
    std::optional<Task> dequeue(
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
    
    /**
     * @brief Peek at highest-priority task without removing
     * 
     * @return Optional task (nullopt if empty)
     * 
     * @details
     * View next task without modifying queue.
     * Useful for:
     * - Checking priority before processing
     * - Monitoring queue state
     * - Conditional dequeue logic
     * 
     * **Time Complexity**: O(1) - constant time
     * 
     * @code
     * auto next = queue.peek();
     * if (next && next->priority == TaskPriority::HIGH) {
     *     // Process high-priority immediately
     *     auto task = queue.dequeue();
     * }
     * @endcode
     * 
     * @warning Task reference may become invalid if queue modified
     * @note Thread-safe but task may be dequeued by another thread
     */
    std::optional<Task> peek() const;
    
    /**
     * @brief Get current number of tasks in queue
     * @return Size of queue
     * 
     * @details
     * **Time Complexity**: O(1) - cached value
     * **Thread Safety**: Fully synchronized
     * 
     * @note Size may change immediately after call (concurrent access)
     */
    size_t size() const;
    
    /**
     * @brief Check if queue is empty
     * @return true if no tasks in queue
     * 
     * @details
     * Equivalent to `size() == 0` but more expressive.
     * 
     * @note State may change immediately after call
     */
    bool empty() const;
    
    /**
     * @brief Check if queue is at capacity
     * @return true if no space for new tasks
     * 
     * @details
     * Returns false if capacity = 0 (unbounded queue).
     * 
     * @note State may change immediately after call
     */
    bool full() const;
    
    /**
     * @brief Get maximum queue capacity
     * @return Max capacity (0 = unbounded)
     */
    size_t capacity() const;
    
    /**
     * @brief Clear all tasks from queue
     * 
     * @details
     * Removes all tasks and notifies waiting producers.
     * Useful for:
     * - Graceful shutdown
     * - Error recovery
     * - Priority change (flush old tasks)
     * 
     * **Time Complexity**: O(n) where n = queue size
     * 
     * @warning All tasks are lost (consider draining first)
     * @note Thread-safe operation
     */
    void clear();
    
    /**
     * @brief Get queue statistics
     * 
     * @return JSON object with statistics
     * 
     * @details
     * Returns:
     * - current_size: Number of tasks
     * - capacity: Max capacity
     * - utilization: Percentage full (0-100)
     * - priority_breakdown: Count per priority level
     * 
     * @code
     * auto stats = queue.get_stats();
     * std::cout << "Queue utilization: " 
     *           << stats["utilization"] << "%" << std::endl;
     * @endcode
     */
    nlohmann::json get_stats() const;

private:
    /**
     * @brief Comparator for priority queue ordering
     * 
     * @details
     * Orders tasks by:
     * 1. Priority (lower enum value = higher priority)
     * 2. Timestamp (earlier = higher priority for same priority level)
     * 
     * This ensures:
     * - HIGH priority processed before MEDIUM/LOW
     * - FIFO ordering within same priority
     * 
     * @note Used by std::priority_queue internally
     */
    struct TaskComparator {
        bool operator()(const Task& a, const Task& b) const {
            if (a.priority != b.priority) {
                // Lower enum value = higher priority (process first)
                return static_cast<int>(a.priority) > static_cast<int>(b.priority);
            }
            // Same priority: earlier timestamp = higher priority (FIFO)
            return a.created_at > b.created_at;
        }
    };
    
    mutable std::mutex mutex_;                    ///< Protects all shared state
    std::condition_variable not_empty_;           ///< Signals when task available
    std::condition_variable not_full_;            ///< Signals when space available
    
    std::priority_queue<Task, 
                        std::vector<Task>, 
                        TaskComparator> queue_;   ///< Underlying priority queue
    
    size_t max_capacity_;                         ///< Maximum queue size (0 = unbounded)
    bool shutdown_ = false;                       ///< Shutdown flag (unblock threads)
};

} // namespace telemetry_processing
