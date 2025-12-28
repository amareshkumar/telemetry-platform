#include "task_queue.h"
#include <algorithm>

namespace telemetry_processing {

TaskQueue::TaskQueue(size_t max_capacity)
    : max_capacity_(max_capacity) {
}

TaskQueue::~TaskQueue() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
    }
    // Unblock all waiting threads
    not_empty_.notify_all();
    not_full_.notify_all();
}

bool TaskQueue::enqueue(Task task, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // Check if queue is shutting down
    if (shutdown_) {
        return false;
    }
    
    // Wait for space if queue is full
    if (max_capacity_ > 0) {
        if (timeout.count() == 0) {
            // Non-blocking: return immediately if full
            if (queue_.size() >= max_capacity_) {
                return false;
            }
        } else {
            // Blocking: wait up to timeout for space
            bool success = not_full_.wait_for(lock, timeout, [this] {
                return shutdown_ || queue_.size() < max_capacity_;
            });
            
            if (!success || shutdown_) {
                return false;
            }
        }
    }
    
    // Add task to queue
    queue_.push(std::move(task));
    
    // Notify waiting consumers
    not_empty_.notify_one();
    
    return true;
}

std::optional<Task> TaskQueue::dequeue(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // Wait for task if queue is empty
    if (timeout.count() == 0) {
        // Non-blocking: return immediately if empty
        if (queue_.empty()) {
            return std::nullopt;
        }
    } else {
        // Blocking: wait up to timeout for task
        bool success = not_empty_.wait_for(lock, timeout, [this] {
            return shutdown_ || !queue_.empty();
        });
        
        if (!success || (shutdown_ && queue_.empty())) {
            return std::nullopt;
        }
    }
    
    // Extract highest-priority task
    Task task = std::move(const_cast<Task&>(queue_.top()));
    queue_.pop();
    
    // Notify waiting producers
    not_full_.notify_one();
    
    return task;
}

std::optional<Task> TaskQueue::peek() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (queue_.empty()) {
        return std::nullopt;
    }
    
    return queue_.top();
}

size_t TaskQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

bool TaskQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

bool TaskQueue::full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return max_capacity_ > 0 && queue_.size() >= max_capacity_;
}

size_t TaskQueue::capacity() const {
    return max_capacity_;
}

void TaskQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Clear all tasks
    while (!queue_.empty()) {
        queue_.pop();
    }
    
    // Notify waiting producers
    not_full_.notify_all();
}

nlohmann::json TaskQueue::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    nlohmann::json stats;
    stats["current_size"] = queue_.size();
    stats["capacity"] = max_capacity_;
    
    // Calculate utilization
    if (max_capacity_ > 0) {
        stats["utilization"] = (queue_.size() * 100.0) / max_capacity_;
    } else {
        stats["utilization"] = 0.0;  // Unbounded queue
    }
    
    // Count tasks by priority (requires copying queue - expensive!)
    // For production, maintain separate counters
    stats["priority_breakdown"] = {
        {"HIGH", 0},
        {"MEDIUM", 0},
        {"LOW", 0}
    };
    
    // Note: Can't iterate std::priority_queue directly
    // This is a limitation - in production, maintain separate counters
    stats["note"] = "Priority breakdown not available (priority_queue limitation)";
    
    return stats;
}

} // namespace telemetry_processing
