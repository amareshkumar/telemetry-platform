#include "telemetry_common/redis_client.h"
#include <sw/redis++/redis++.h>
#include <stdexcept>
#include <sstream>

namespace telemetry_common {

// ========== Constructor & Destructor ==========

RedisClient::RedisClient(const ConnectionOptions& options)
    : options_(options)
{
    try {
        // Build connection string
        sw::redis::ConnectionOptions conn_opts;
        conn_opts.host = options_.host;
        conn_opts.port = options_.port;
        conn_opts.password = options_.password;
        conn_opts.db = options_.db;
        conn_opts.connect_timeout = options_.connect_timeout;
        conn_opts.socket_timeout = options_.socket_timeout;

        // Connection pool options
        sw::redis::ConnectionPoolOptions pool_opts;
        pool_opts.size = options_.pool_size;

        // Create Redis client with connection pooling
        redis_ = std::make_unique<sw::redis::Redis>(conn_opts, pool_opts);

        // Test connection
        if (!ping()) {
            throw std::runtime_error("Failed to connect to Redis server");
        }
    }
    catch (const sw::redis::Error& e) {
        std::ostringstream oss;
        oss << "Redis connection error: " << e.what();
        throw std::runtime_error(oss.str());
    }
}

RedisClient::~RedisClient() = default;

// Move constructor and assignment
RedisClient::RedisClient(RedisClient&&) noexcept = default;
RedisClient& RedisClient::operator=(RedisClient&&) noexcept = default;

// ========== Connection Management ==========

bool RedisClient::ping() {
    if (!redis_) return false;
    try {
        std::string response = redis_->ping();
        return response == "PONG";
    }
    catch (const sw::redis::Error&) {
        return false;
    }
}

bool RedisClient::is_connected() const {
    return redis_ != nullptr;
}

// ========== String Operations ==========

bool RedisClient::set(const std::string& key, const std::string& value, int ttl_seconds) {
    if (!redis_) return false;
    try {
        if (ttl_seconds > 0) {
            auto ttl = std::chrono::seconds(ttl_seconds);
            return redis_->set(key, value, ttl);
        } else {
            return redis_->set(key, value);
        }
    }
    catch (const sw::redis::Error&) {
        return false;
    }
}

std::optional<std::string> RedisClient::get(const std::string& key) {
    if (!redis_) return std::nullopt;
    try {
        auto val = redis_->get(key);
        if (val) {
            return *val;
        }
        return std::nullopt;
    }
    catch (const sw::redis::Error&) {
        return std::nullopt;
    }
}

int RedisClient::del(const std::string& key) {
    if (!redis_) return 0;
    try {
        return static_cast<int>(redis_->del(key));
    }
    catch (const sw::redis::Error&) {
        return 0;
    }
}

bool RedisClient::exists(const std::string& key) {
    if (!redis_) return false;
    try {
        return redis_->exists(key) > 0;
    }
    catch (const sw::redis::Error&) {
        return false;
    }
}

bool RedisClient::expire(const std::string& key, int seconds) {
    if (!redis_) return false;
    try {
        return redis_->expire(key, std::chrono::seconds(seconds));
    }
    catch (const sw::redis::Error&) {
        return false;
    }
}

int RedisClient::ttl(const std::string& key) {
    if (!redis_) return -2;
    try {
        // redis++: ttl() returns long long (seconds)
        long long ttl_seconds = redis_->ttl(key);
        return static_cast<int>(ttl_seconds);
    }
    catch (const sw::redis::Error&) {
        return -2;
    }
}

// ========== List Operations ==========

long long RedisClient::lpush(const std::string& key, const std::string& value) {
    if (!redis_) return 0;
    try {
        return redis_->lpush(key, value);
    }
    catch (const sw::redis::Error&) {
        return 0;
    }
}

std::optional<std::string> RedisClient::rpop(const std::string& key) {
    if (!redis_) return std::nullopt;
    try {
        auto val = redis_->rpop(key);
        if (val) {
            return *val;
        }
        return std::nullopt;
    }
    catch (const sw::redis::Error&) {
        return std::nullopt;
    }
}

std::optional<std::string> RedisClient::brpop(const std::string& key, int timeout_seconds) {
    if (!redis_) return std::nullopt;
    try {
        std::pair<std::string, std::string> result;
        if (timeout_seconds > 0) {
            auto timeout = std::chrono::seconds(timeout_seconds);
            auto val = redis_->brpop(key, timeout);
            if (val) {
                return val->second;  // Return value, not key
            }
        } else {
            // Block indefinitely
            auto val = redis_->brpop(key, std::chrono::seconds(0));
            if (val) {
                return val->second;
            }
        }
        return std::nullopt;
    }
    catch (const sw::redis::Error&) {
        return std::nullopt;
    }
}

long long RedisClient::llen(const std::string& key) {
    if (!redis_) return 0;
    try {
        return redis_->llen(key);
    }
    catch (const sw::redis::Error&) {
        return 0;
    }
}

std::vector<std::string> RedisClient::lrange(const std::string& key, long long start, long long stop) {
    if (!redis_) return {};
    try {
        std::vector<std::string> result;
        redis_->lrange(key, start, stop, std::back_inserter(result));
        return result;
    }
    catch (const sw::redis::Error&) {
        return {};
    }
}

// ========== Set Operations ==========

long long RedisClient::sadd(const std::string& key, const std::string& member) {
    if (!redis_) return 0;
    try {
        return redis_->sadd(key, member);
    }
    catch (const sw::redis::Error&) {
        return 0;
    }
}

bool RedisClient::sismember(const std::string& key, const std::string& member) {
    if (!redis_) return false;
    try {
        return redis_->sismember(key, member);
    }
    catch (const sw::redis::Error&) {
        return false;
    }
}

long long RedisClient::srem(const std::string& key, const std::string& member) {
    if (!redis_) return 0;
    try {
        return redis_->srem(key, member);
    }
    catch (const sw::redis::Error&) {
        return 0;
    }
}

// ========== Sorted Set Operations ==========

long long RedisClient::zadd(const std::string& key, const std::string& member, double score) {
    if (!redis_) return 0;
    try {
        return redis_->zadd(key, member, score);
    }
    catch (const sw::redis::Error&) {
        return 0;
    }
}

std::optional<std::string> RedisClient::zpopmax(const std::string& key) {
    if (!redis_) return std::nullopt;
    try {
        std::pair<std::string, double> result;
        auto val = redis_->zpopmax(key);
        if (val) {
            return val->first;  // Return member, not score
        }
        return std::nullopt;
    }
    catch (const sw::redis::Error&) {
        return std::nullopt;
    }
}

long long RedisClient::zcard(const std::string& key) {
    if (!redis_) return 0;
    try {
        return redis_->zcard(key);
    }
    catch (const sw::redis::Error&) {
        return 0;
    }
}

// ========== Atomic Operations ==========

long long RedisClient::incr(const std::string& key) {
    if (!redis_) return 0;
    try {
        return redis_->incr(key);
    }
    catch (const sw::redis::Error&) {
        return 0;
    }
}

long long RedisClient::decr(const std::string& key) {
    if (!redis_) return 0;
    try {
        return redis_->decr(key);
    }
    catch (const sw::redis::Error&) {
        return 0;
    }
}

// ========== Statistics & Debugging ==========

std::string RedisClient::info() {
    if (!redis_) return "";
    try {
        return redis_->info();
    }
    catch (const sw::redis::Error&) {
        return "";
    }
}

} // namespace telemetry_common
