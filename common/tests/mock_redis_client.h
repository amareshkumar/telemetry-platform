#pragma once

#include "telemetry_common/i_redis_client.h"
#include <gmock/gmock.h>

namespace telemetry_common {
namespace testing {

/**
 * MockRedisClient - Google Mock implementation of IRedisClient
 * 
 * Used for unit testing without real Redis server.
 * Provides full control over behavior using gmock expectations.
 * 
 * Interview Talking Points:
 * - Google Mock: Industry-standard mocking framework for C++
 * - MOCK_METHOD: Generates mock functions with call expectations
 * - Expectations: EXPECT_CALL sets up return values and verifies calls
 * - Test isolation: Each test can configure mock behavior independently
 * 
 * Example usage:
 *   MockRedisClient mock;
 *   EXPECT_CALL(mock, get("key")).WillOnce(Return(std::optional<std::string>("value")));
 *   auto result = mock.get("key");  // Returns "value"
 */
class MockRedisClient : public IRedisClient {
public:
    // Connection management
    MOCK_METHOD(bool, ping, (), (const, override));

    // String operations
    MOCK_METHOD(bool, set, (const std::string& key, const std::string& value), (override));
    MOCK_METHOD(std::optional<std::string>, get, (const std::string& key), (override));
    MOCK_METHOD(int64_t, del, (const std::string& key), (override));
    MOCK_METHOD(int64_t, del, (const std::vector<std::string>& keys), (override));
    MOCK_METHOD(bool, exists, (const std::string& key), (override));
    MOCK_METHOD(bool, expire, (const std::string& key, int64_t seconds), (override));
    MOCK_METHOD(std::optional<int64_t>, ttl, (const std::string& key), (override));

    // List operations
    MOCK_METHOD(int64_t, lpush, (const std::string& key, const std::string& value), (override));
    MOCK_METHOD(std::optional<std::string>, rpop, (const std::string& key), (override));
    MOCK_METHOD(std::optional<std::string>, brpop, (const std::string& key, int timeout_sec), (override));
    MOCK_METHOD(int64_t, llen, (const std::string& key), (override));
    MOCK_METHOD(std::vector<std::string>, lrange, (const std::string& key, int64_t start, int64_t stop), (override));

    // Set operations
    MOCK_METHOD(int64_t, sadd, (const std::string& key, const std::string& member), (override));
    MOCK_METHOD(bool, sismember, (const std::string& key, const std::string& member), (override));
    MOCK_METHOD(int64_t, srem, (const std::string& key, const std::string& member), (override));
    MOCK_METHOD(int64_t, scard, (const std::string& key), (override));

    // Sorted set operations
    MOCK_METHOD(bool, zadd, (const std::string& key, double score, const std::string& member), (override));
    MOCK_METHOD((std::optional<std::pair<std::string, double>>), zpopmax, (const std::string& key), (override));
    MOCK_METHOD(int64_t, zcard, (const std::string& key), (override));

    // Atomic operations
    MOCK_METHOD(int64_t, incr, (const std::string& key), (override));
    MOCK_METHOD(int64_t, decr, (const std::string& key), (override));
};

} // namespace testing
} // namespace telemetry_common
