#include "telemetry_processor/RedisClient.h"
#include <map>
#include <deque>
#include <mutex>

namespace telemetry_processor {

/**
 * Day 1: Mock Redis implementation using in-memory data structures
 * This allows us to build and test without external Redis dependency
 * 
 * Day 2: Replace with real redis-plus-plus implementation
 */
struct RedisClient::Impl {
    std::map<std::string, std::string> kv_store;           // Key-value store
    std::map<std::string, std::deque<std::string>> lists;  // Lists
    std::mutex mutex;                                       // Thread safety
    
    bool rpush(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex);
        lists[key].push_back(value);
        return true;
    }
    
    std::optional<std::string> blpop(const std::string& key, int /*timeout_seconds*/) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = lists.find(key);
        if (it == lists.end() || it->second.empty()) {
            return std::nullopt;
        }
        
        std::string value = it->second.front();
        it->second.pop_front();
        return value;
    }
    
    bool set(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(mutex);
        kv_store[key] = value;
        return true;
    }
    
    std::optional<std::string> get(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = kv_store.find(key);
        if (it == kv_store.end()) {
            return std::nullopt;
        }
        return it->second;
    }
    
    bool del(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex);
        bool deleted = kv_store.erase(key) > 0;
        deleted |= lists.erase(key) > 0;
        return deleted;
    }
    
    long long llen(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = lists.find(key);
        if (it == lists.end()) {
            return 0;
        }
        return static_cast<long long>(it->second.size());
    }
};

RedisClient::RedisClient(const std::string& host, int port)
    : host_(host)
    , port_(port)
    , connected_(false)
    , impl_(std::make_unique<Impl>())
{
}

RedisClient::~RedisClient() = default;

bool RedisClient::connect() {
    // Mock: always succeeds
    connected_ = true;
    return true;
}

bool RedisClient::is_connected() const {
    return connected_;
}

std::string RedisClient::ping() {
    if (!connected_) {
        return "";
    }
    return "PONG";
}

bool RedisClient::rpush(const std::string& key, const std::string& value) {
    if (!connected_) {
        return false;
    }
    return impl_->rpush(key, value);
}

std::optional<std::string> RedisClient::blpop(const std::string& key, int timeout_seconds) {
    if (!connected_) {
        return std::nullopt;
    }
    return impl_->blpop(key, timeout_seconds);
}

bool RedisClient::set(const std::string& key, const std::string& value) {
    if (!connected_) {
        return false;
    }
    return impl_->set(key, value);
}

std::optional<std::string> RedisClient::get(const std::string& key) {
    if (!connected_) {
        return std::nullopt;
    }
    return impl_->get(key);
}

bool RedisClient::del(const std::string& key) {
    if (!connected_) {
        return false;
    }
    return impl_->del(key);
}

long long RedisClient::llen(const std::string& key) {
    if (!connected_) {
        return -1;
    }
    return impl_->llen(key);
}

} // namespace telemetry_processor
