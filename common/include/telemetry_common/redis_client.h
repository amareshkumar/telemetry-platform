#pragma once

#include <string>
#include <optional>
#include <vector>
#include <chrono>
#include <memory>

// Forward declarations to avoid exposing redis++ in header
namespace sw {
namespace redis {
    class Redis;
}
}

namespace telemetry_common {

/**
 * @brief Redis client wrapper providing RAII and exception-safe operations
 * 
 * This class wraps redis-plus-plus library and provides:
 * - RAII resource management (connection cleanup)
 * - Exception-safe operations
 * - Common Redis operations (strings, lists, sets)
 * - Connection pooling support
 * 
 * Interview talking points:
 * - Why redis-plus-plus? Modern C++ API with RAII, exception safety
 * - Connection pooling for production deployments
 * - Automatic reconnection on network failures
 */
class RedisClient {
public:
    /**
     * @brief Connection options for Redis
     */
    struct ConnectionOptions {
        std::string host = "localhost";
        int port = 6379;
        std::string password = "";
        int db = 0;                           // Database index (0-15)
        int pool_size = 5;                    // Connection pool size
        std::chrono::milliseconds connect_timeout{1000};
        std::chrono::milliseconds socket_timeout{1000};
    };

    /**
     * @brief Construct Redis client with connection options
     * @param options Connection configuration
     * @throws std::runtime_error if connection fails
     */
    explicit RedisClient(const ConnectionOptions& options = ConnectionOptions{});

    /**
     * @brief Destructor - automatically closes connection
     */
    ~RedisClient();

    // Disable copy (Redis connection is not copyable)
    RedisClient(const RedisClient&) = delete;
    RedisClient& operator=(const RedisClient&) = delete;

    // Enable move
    RedisClient(RedisClient&&) noexcept;
    RedisClient& operator=(RedisClient&&) noexcept;

    // ========== Connection Management ==========

    /**
     * @brief Test connection with PING command
     * @return true if connection is alive
     */
    bool ping();

    /**
     * @brief Check if client is connected
     */
    bool is_connected() const;

    // ========== String Operations (Key-Value) ==========

    /**
     * @brief Set a key-value pair
     * @param key The key
     * @param value The value
     * @param ttl_seconds Time-to-live in seconds (0 = no expiration)
     * @return true if successful
     */
    bool set(const std::string& key, const std::string& value, int ttl_seconds = 0);

    /**
     * @brief Get value by key
     * @param key The key
     * @return Value if exists, std::nullopt otherwise
     */
    std::optional<std::string> get(const std::string& key);

    /**
     * @brief Delete a key
     * @param key The key to delete
     * @return Number of keys deleted (0 or 1)
     */
    int del(const std::string& key);

    /**
     * @brief Check if key exists
     * @param key The key
     * @return true if key exists
     */
    bool exists(const std::string& key);

    /**
     * @brief Set expiration on a key
     * @param key The key
     * @param seconds TTL in seconds
     * @return true if successful
     */
    bool expire(const std::string& key, int seconds);

    /**
     * @brief Get time-to-live for a key
     * @param key The key
     * @return TTL in seconds, -1 if no expiration, -2 if key doesn't exist
     */
    int ttl(const std::string& key);

    // ========== List Operations (Task Queue) ==========

    /**
     * @brief Push element to head of list (left push)
     * @param key List key
     * @param value Value to push
     * @return Length of list after push
     * 
     * Interview note: LPUSH is O(1), perfect for producer side of queue
     */
    long long lpush(const std::string& key, const std::string& value);

    /**
     * @brief Pop element from tail of list (right pop)
     * @param key List key
     * @return Value if list not empty, std::nullopt otherwise
     * 
     * Interview note: RPOP is O(1), perfect for consumer side of queue
     * Combined with LPUSH creates FIFO queue
     */
    std::optional<std::string> rpop(const std::string& key);

    /**
     * @brief Blocking right pop (waits for element)
     * @param key List key
     * @param timeout_seconds Timeout in seconds (0 = block indefinitely)
     * @return Value if available within timeout, std::nullopt otherwise
     * 
     * Interview note: BRPOP blocks worker thread until task available,
     * more efficient than polling RPOP in loop
     */
    std::optional<std::string> brpop(const std::string& key, int timeout_seconds = 0);

    /**
     * @brief Get list length
     * @param key List key
     * @return Number of elements in list
     */
    long long llen(const std::string& key);

    /**
     * @brief Get range of elements from list
     * @param key List key
     * @param start Start index (0-based)
     * @param stop Stop index (-1 = end)
     * @return Vector of elements
     */
    std::vector<std::string> lrange(const std::string& key, long long start, long long stop);

    // ========== Set Operations (Task Deduplication) ==========

    /**
     * @brief Add member to set
     * @param key Set key
     * @param member Member to add
     * @return Number of members added (0 if already exists, 1 if new)
     */
    long long sadd(const std::string& key, const std::string& member);

    /**
     * @brief Check if member exists in set
     * @param key Set key
     * @param member Member to check
     * @return true if member exists
     */
    bool sismember(const std::string& key, const std::string& member);

    /**
     * @brief Remove member from set
     * @param key Set key
     * @param member Member to remove
     * @return Number of members removed
     */
    long long srem(const std::string& key, const std::string& member);

    // ========== Sorted Set Operations (Priority Queue) ==========

    /**
     * @brief Add member to sorted set with score
     * @param key Sorted set key
     * @param member Member to add
     * @param score Score for ordering (higher = higher priority)
     * @return Number of members added
     * 
     * Interview note: Sorted sets enable priority queue implementation.
     * Score can represent priority level or timestamp.
     */
    long long zadd(const std::string& key, const std::string& member, double score);

    /**
     * @brief Pop member with highest score (max)
     * @param key Sorted set key
     * @return Member if exists, std::nullopt otherwise
     * 
     * Interview note: ZPOPMAX pops highest priority task first
     */
    std::optional<std::string> zpopmax(const std::string& key);

    /**
     * @brief Get sorted set cardinality (size)
     * @param key Sorted set key
     * @return Number of members
     */
    long long zcard(const std::string& key);

    // ========== Atomic Operations ==========

    /**
     * @brief Atomic increment
     * @param key The key
     * @return New value after increment
     */
    long long incr(const std::string& key);

    /**
     * @brief Atomic decrement
     * @param key The key
     * @return New value after decrement
     */
    long long decr(const std::string& key);

    // ========== Statistics & Debugging ==========

    /**
     * @brief Get connection options used
     */
    const ConnectionOptions& get_options() const { return options_; }

    /**
     * @brief Get info about Redis server
     * @return Server info string
     */
    std::string info();

private:
    ConnectionOptions options_;
    std::unique_ptr<sw::redis::Redis> redis_;  // PIMPL pattern to hide implementation
};

} // namespace telemetry_common
