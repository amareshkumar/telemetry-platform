#pragma once

#include <string>
#include <optional>
#include <vector>
#include <memory>

namespace telemetry_processor {

/**
 * @brief Redis client wrapper for DistQueue
 * 
 * Day 1: Simple interface, mock implementation
 * Day 2+: Real Redis integration with redis-plus-plus
 */
class RedisClient {
public:
    /**
     * @brief Constructor
     * @param host Redis server host
     * @param port Redis server port
     */
    RedisClient(const std::string& host = "127.0.0.1", int port = 6379);
    
    /**
     * @brief Destructor
     */
    ~RedisClient();
    
    /**
     * @brief Connect to Redis server
     * @return true if successful
     */
    bool connect();
    
    /**
     * @brief Check if connected
     */
    bool is_connected() const;
    
    /**
     * @brief Ping Redis server
     * @return "PONG" if successful, empty if failed
     */
    std::string ping();
    
    /**
     * @brief Push item to right of list (RPUSH)
     * @param key List key
     * @param value Value to push
     * @return true if successful
     */
    bool rpush(const std::string& key, const std::string& value);
    
    /**
     * @brief Blocking left pop from list (BLPOP)
     * @param key List key
     * @param timeout_seconds Timeout in seconds (0 = wait forever)
     * @return Popped value, or nullopt if timeout
     */
    std::optional<std::string> blpop(const std::string& key, int timeout_seconds = 0);
    
    /**
     * @brief Set key-value pair (SET)
     * @param key Key
     * @param value Value
     * @return true if successful
     */
    bool set(const std::string& key, const std::string& value);
    
    /**
     * @brief Get value by key (GET)
     * @param key Key
     * @return Value if exists, nullopt otherwise
     */
    std::optional<std::string> get(const std::string& key);
    
    /**
     * @brief Delete key (DEL)
     * @param key Key to delete
     * @return true if key was deleted
     */
    bool del(const std::string& key);
    
    /**
     * @brief Get list length (LLEN)
     * @param key List key
     * @return List length, -1 on error
     */
    long long llen(const std::string& key);

private:
    std::string host_;
    int port_;
    bool connected_;
    
    // Redis connection (will add real redis++ connection later)
    // For Day 1, we'll use an in-memory mock
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace telemetry_processor
