#pragma once

#include <string>
#include <optional>
#include <vector>
#include <chrono>

namespace telemetry_common {

/**
 * IRedisClient - Abstract interface for Redis operations
 * 
 * This interface enables dependency injection and testing with Google Mock.
 * Production code uses RedisClient (real redis-plus-plus implementation).
 * Tests use MockRedisClient (gmock implementation).
 * 
 * Interview Talking Point:
 * - Dependency Inversion Principle: Depend on abstractions, not concrete implementations
 * - Enables testing: Inject mock in tests, real client in production
 * - Interface segregation: Only methods we actually use
 */
class IRedisClient {
public:
    virtual ~IRedisClient() = default;

    // Connection management
    virtual bool ping() const = 0;

    // String operations
    virtual bool set(const std::string& key, const std::string& value) = 0;
    virtual std::optional<std::string> get(const std::string& key) = 0;
    virtual int64_t del(const std::string& key) = 0;
    virtual int64_t del(const std::vector<std::string>& keys) = 0;
    virtual bool exists(const std::string& key) = 0;
    virtual bool expire(const std::string& key, int64_t seconds) = 0;
    virtual std::optional<int64_t> ttl(const std::string& key) = 0;

    // List operations (FIFO queue)
    virtual int64_t lpush(const std::string& key, const std::string& value) = 0;
    virtual std::optional<std::string> rpop(const std::string& key) = 0;
    virtual std::optional<std::string> brpop(const std::string& key, int timeout_sec) = 0;
    virtual int64_t llen(const std::string& key) = 0;
    virtual std::vector<std::string> lrange(const std::string& key, int64_t start, int64_t stop) = 0;

    // Set operations (deduplication)
    virtual int64_t sadd(const std::string& key, const std::string& member) = 0;
    virtual bool sismember(const std::string& key, const std::string& member) = 0;
    virtual int64_t srem(const std::string& key, const std::string& member) = 0;
    virtual int64_t scard(const std::string& key) = 0;

    // Sorted set operations (priority queue)
    virtual bool zadd(const std::string& key, double score, const std::string& member) = 0;
    virtual std::optional<std::pair<std::string, double>> zpopmax(const std::string& key) = 0;
    virtual int64_t zcard(const std::string& key) = 0;

    // Atomic operations
    virtual int64_t incr(const std::string& key) = 0;
    virtual int64_t decr(const std::string& key) = 0;
};

} // namespace telemetry_common
