// Day 2: Redis Client Unit Tests with Google Test & Google Mock
//
// Interview Talking Points:
// - Google Test: Industry-standard testing framework (used by Google, Microsoft, AWS)
// - Google Mock: Mocking framework for C++ (expectations, matchers, actions)
// - Test fixtures: Setup/teardown with shared state
// - EXPECT vs ASSERT: EXPECT continues test, ASSERT stops on failure
// - Matchers: Eq, Optional, IsEmpty, etc. for readable assertions

#include "mock_redis_client.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace telemetry_common;
using namespace telemetry_common::testing;
using ::testing::Return;
using ::testing::Eq;
using ::testing::Optional;
using ::testing::IsEmpty;
using ::testing::SizeIs;

// ============================================================================
// Test Fixture for Redis Client Tests
// ============================================================================

class RedisClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup runs before each test
        mock_redis_ = std::make_unique<MockRedisClient>();
    }

    void TearDown() override {
        // Teardown runs after each test
        mock_redis_.reset();
    }

    std::unique_ptr<MockRedisClient> mock_redis_;
};

// ============================================================================
// Test Suite 1: Connection Management
// ============================================================================

TEST_F(RedisClientTest, PingReturnsTrue) {
    EXPECT_CALL(*mock_redis_, ping())
        .WillOnce(Return(true));

    EXPECT_TRUE(mock_redis_->ping());
}

TEST_F(RedisClientTest, PingReturnsFalseWhenDisconnected) {
    EXPECT_CALL(*mock_redis_, ping())
        .WillOnce(Return(false));

    EXPECT_FALSE(mock_redis_->ping());
}

// ============================================================================
// Test Suite 2: String Operations (CRUD)
// ============================================================================

TEST_F(RedisClientTest, SetReturnsTrue) {
    EXPECT_CALL(*mock_redis_, set("user:1:name", "Amaresh"))
        .WillOnce(Return(true));

    EXPECT_TRUE(mock_redis_->set("user:1:name", "Amaresh"));
}

TEST_F(RedisClientTest, GetReturnsValue) {
    EXPECT_CALL(*mock_redis_, get("user:1:name"))
        .WillOnce(Return(std::optional<std::string>("Amaresh")));

    auto result = mock_redis_->get("user:1:name");
    EXPECT_THAT(result, Optional(Eq("Amaresh")));
}

TEST_F(RedisClientTest, GetReturnsNulloptForMissingKey) {
    EXPECT_CALL(*mock_redis_, get("missing:key"))
        .WillOnce(Return(std::nullopt));

    auto result = mock_redis_->get("missing:key");
    EXPECT_FALSE(result.has_value());
}

TEST_F(RedisClientTest, DelReturnsCount) {
    EXPECT_CALL(*mock_redis_, del("key1"))
        .WillOnce(Return(1));

    EXPECT_EQ(mock_redis_->del("key1"), 1);
}

TEST_F(RedisClientTest, DelMultipleKeys) {
    std::vector<std::string> keys = {"key1", "key2", "key3"};
    EXPECT_CALL(*mock_redis_, del(keys))
        .WillOnce(Return(3));

    EXPECT_EQ(mock_redis_->del(keys), 3);
}

TEST_F(RedisClientTest, ExistsReturnsTrue) {
    EXPECT_CALL(*mock_redis_, exists("existing_key"))
        .WillOnce(Return(true));

    EXPECT_TRUE(mock_redis_->exists("existing_key"));
}

TEST_F(RedisClientTest, ExistsReturnsFalse) {
    EXPECT_CALL(*mock_redis_, exists("missing_key"))
        .WillOnce(Return(false));

    EXPECT_FALSE(mock_redis_->exists("missing_key"));
}

// ============================================================================
// Test Suite 3: Expiration
// ============================================================================

TEST_F(RedisClientTest, ExpireReturnsTrue) {
    EXPECT_CALL(*mock_redis_, expire("temp_key", 10))
        .WillOnce(Return(true));

    EXPECT_TRUE(mock_redis_->expire("temp_key", 10));
}

TEST_F(RedisClientTest, TtlReturnsRemainingTime) {
    EXPECT_CALL(*mock_redis_, ttl("temp_key"))
        .WillOnce(Return(std::optional<int64_t>(9)));

    auto result = mock_redis_->ttl("temp_key");
    EXPECT_THAT(result, Optional(Eq(9)));
}

TEST_F(RedisClientTest, TtlReturnsMinusOneForNoExpiration) {
    EXPECT_CALL(*mock_redis_, ttl("permanent_key"))
        .WillOnce(Return(std::optional<int64_t>(-1)));

    auto result = mock_redis_->ttl("permanent_key");
    EXPECT_THAT(result, Optional(Eq(-1)));
}

TEST_F(RedisClientTest, TtlReturnsMinusTwoForMissingKey) {
    EXPECT_CALL(*mock_redis_, ttl("missing_key"))
        .WillOnce(Return(std::optional<int64_t>(-2)));

    auto result = mock_redis_->ttl("missing_key");
    EXPECT_THAT(result, Optional(Eq(-2)));
}

// ============================================================================
// Test Suite 4: List Operations (Task Queue)
// ============================================================================

TEST_F(RedisClientTest, LpushReturnsLength) {
    EXPECT_CALL(*mock_redis_, lpush("tasks", "task1"))
        .WillOnce(Return(1));

    EXPECT_EQ(mock_redis_->lpush("tasks", "task1"), 1);
}

TEST_F(RedisClientTest, RpopReturnsValue) {
    EXPECT_CALL(*mock_redis_, rpop("tasks"))
        .WillOnce(Return(std::optional<std::string>("task1")));

    auto result = mock_redis_->rpop("tasks");
    EXPECT_THAT(result, Optional(Eq("task1")));
}

TEST_F(RedisClientTest, RpopReturnsNulloptForEmptyQueue) {
    EXPECT_CALL(*mock_redis_, rpop("empty_queue"))
        .WillOnce(Return(std::nullopt));

    auto result = mock_redis_->rpop("empty_queue");
    EXPECT_FALSE(result.has_value());
}

TEST_F(RedisClientTest, BrpopReturnsValue) {
    EXPECT_CALL(*mock_redis_, brpop("tasks", 5))
        .WillOnce(Return(std::optional<std::string>("task1")));

    auto result = mock_redis_->brpop("tasks", 5);
    EXPECT_THAT(result, Optional(Eq("task1")));
}

TEST_F(RedisClientTest, LlenReturnsQueueLength) {
    EXPECT_CALL(*mock_redis_, llen("tasks"))
        .WillOnce(Return(5));

    EXPECT_EQ(mock_redis_->llen("tasks"), 5);
}

TEST_F(RedisClientTest, LrangeReturnsItems) {
    std::vector<std::string> items = {"task1", "task2", "task3"};
    EXPECT_CALL(*mock_redis_, lrange("tasks", 0, -1))
        .WillOnce(Return(items));

    auto result = mock_redis_->lrange("tasks", 0, -1);
    EXPECT_THAT(result, SizeIs(3));
    EXPECT_EQ(result[0], "task1");
    EXPECT_EQ(result[1], "task2");
    EXPECT_EQ(result[2], "task3");
}

TEST_F(RedisClientTest, LrangeReturnsEmptyForMissingList) {
    EXPECT_CALL(*mock_redis_, lrange("missing_list", 0, -1))
        .WillOnce(Return(std::vector<std::string>{}));

    auto result = mock_redis_->lrange("missing_list", 0, -1);
    EXPECT_THAT(result, IsEmpty());
}

// ============================================================================
// Test Suite 5: Set Operations (Deduplication)
// ============================================================================

TEST_F(RedisClientTest, SaddReturnsOneForNewMember) {
    EXPECT_CALL(*mock_redis_, sadd("processed_ids", "id123"))
        .WillOnce(Return(1));

    EXPECT_EQ(mock_redis_->sadd("processed_ids", "id123"), 1);
}

TEST_F(RedisClientTest, SaddReturnsZeroForExistingMember) {
    EXPECT_CALL(*mock_redis_, sadd("processed_ids", "id123"))
        .WillOnce(Return(0));

    EXPECT_EQ(mock_redis_->sadd("processed_ids", "id123"), 0);
}

TEST_F(RedisClientTest, SismemberReturnsTrue) {
    EXPECT_CALL(*mock_redis_, sismember("processed_ids", "id123"))
        .WillOnce(Return(true));

    EXPECT_TRUE(mock_redis_->sismember("processed_ids", "id123"));
}

TEST_F(RedisClientTest, SismemberReturnsFalse) {
    EXPECT_CALL(*mock_redis_, sismember("processed_ids", "id999"))
        .WillOnce(Return(false));

    EXPECT_FALSE(mock_redis_->sismember("processed_ids", "id999"));
}

TEST_F(RedisClientTest, SremReturnsOne) {
    EXPECT_CALL(*mock_redis_, srem("my_set", "member1"))
        .WillOnce(Return(1));

    EXPECT_EQ(mock_redis_->srem("my_set", "member1"), 1);
}

TEST_F(RedisClientTest, ScardReturnsSetSize) {
    EXPECT_CALL(*mock_redis_, scard("my_set"))
        .WillOnce(Return(5));

    EXPECT_EQ(mock_redis_->scard("my_set"), 5);
}

// ============================================================================
// Test Suite 6: Sorted Set Operations (Priority Queue)
// ============================================================================

TEST_F(RedisClientTest, ZaddReturnsTrue) {
    EXPECT_CALL(*mock_redis_, zadd("priority_queue", 100.0, "high_priority_task"))
        .WillOnce(Return(true));

    EXPECT_TRUE(mock_redis_->zadd("priority_queue", 100.0, "high_priority_task"));
}

TEST_F(RedisClientTest, ZpopmaxReturnsHighestPriority) {
    auto expected = std::make_pair("high_priority_task", 100.0);
    EXPECT_CALL(*mock_redis_, zpopmax("priority_queue"))
        .WillOnce(Return(std::optional(expected)));

    auto result = mock_redis_->zpopmax("priority_queue");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->first, "high_priority_task");
    EXPECT_EQ(result->second, 100.0);
}

TEST_F(RedisClientTest, ZpopmaxReturnsNulloptForEmptySet) {
    EXPECT_CALL(*mock_redis_, zpopmax("empty_zset"))
        .WillOnce(Return(std::nullopt));

    auto result = mock_redis_->zpopmax("empty_zset");
    EXPECT_FALSE(result.has_value());
}

TEST_F(RedisClientTest, ZcardReturnsSetSize) {
    EXPECT_CALL(*mock_redis_, zcard("priority_queue"))
        .WillOnce(Return(10));

    EXPECT_EQ(mock_redis_->zcard("priority_queue"), 10);
}

// ============================================================================
// Test Suite 7: Atomic Operations
// ============================================================================

TEST_F(RedisClientTest, IncrReturnsIncrementedValue) {
    EXPECT_CALL(*mock_redis_, incr("counter"))
        .WillOnce(Return(1))
        .WillOnce(Return(2))
        .WillOnce(Return(3));

    EXPECT_EQ(mock_redis_->incr("counter"), 1);
    EXPECT_EQ(mock_redis_->incr("counter"), 2);
    EXPECT_EQ(mock_redis_->incr("counter"), 3);
}

TEST_F(RedisClientTest, DecrReturnsDecrementedValue) {
    EXPECT_CALL(*mock_redis_, decr("counter"))
        .WillOnce(Return(2))
        .WillOnce(Return(1))
        .WillOnce(Return(0));

    EXPECT_EQ(mock_redis_->decr("counter"), 2);
    EXPECT_EQ(mock_redis_->decr("counter"), 1);
    EXPECT_EQ(mock_redis_->decr("counter"), 0);
}

// ============================================================================
// Test Suite 8: Complex Scenarios
// ============================================================================

TEST_F(RedisClientTest, TaskQueueWorkflow) {
    // Producer enqueues tasks
    EXPECT_CALL(*mock_redis_, lpush("pending_tasks", "task_1"))
        .WillOnce(Return(1));
    EXPECT_CALL(*mock_redis_, lpush("pending_tasks", "task_2"))
        .WillOnce(Return(2));
    EXPECT_CALL(*mock_redis_, lpush("pending_tasks", "task_3"))
        .WillOnce(Return(3));

    mock_redis_->lpush("pending_tasks", "task_1");
    mock_redis_->lpush("pending_tasks", "task_2");
    mock_redis_->lpush("pending_tasks", "task_3");

    // Check queue length
    EXPECT_CALL(*mock_redis_, llen("pending_tasks"))
        .WillOnce(Return(3));
    EXPECT_EQ(mock_redis_->llen("pending_tasks"), 3);

    // Consumer processes tasks
    EXPECT_CALL(*mock_redis_, rpop("pending_tasks"))
        .WillOnce(Return(std::optional<std::string>("task_1")))
        .WillOnce(Return(std::optional<std::string>("task_2")));

    auto task1 = mock_redis_->rpop("pending_tasks");
    auto task2 = mock_redis_->rpop("pending_tasks");
    
    EXPECT_THAT(task1, Optional(Eq("task_1")));
    EXPECT_THAT(task2, Optional(Eq("task_2")));

    // Mark as processed
    EXPECT_CALL(*mock_redis_, sadd("processed_tasks", "task_1"))
        .WillOnce(Return(1));
    EXPECT_CALL(*mock_redis_, sadd("processed_tasks", "task_2"))
        .WillOnce(Return(1));

    mock_redis_->sadd("processed_tasks", "task_1");
    mock_redis_->sadd("processed_tasks", "task_2");
}

TEST_F(RedisClientTest, PriorityTaskScheduling) {
    // Add tasks with different priorities
    EXPECT_CALL(*mock_redis_, zadd("task_priorities", 100.0, "critical_task"))
        .WillOnce(Return(true));
    EXPECT_CALL(*mock_redis_, zadd("task_priorities", 10.0, "low_priority_task"))
        .WillOnce(Return(true));

    mock_redis_->zadd("task_priorities", 100.0, "critical_task");
    mock_redis_->zadd("task_priorities", 10.0, "low_priority_task");

    // Pop highest priority first
    EXPECT_CALL(*mock_redis_, zpopmax("task_priorities"))
        .WillOnce(Return(std::optional(std::make_pair("critical_task", 100.0))))
        .WillOnce(Return(std::optional(std::make_pair("low_priority_task", 10.0))));

    auto critical = mock_redis_->zpopmax("task_priorities");
    auto low = mock_redis_->zpopmax("task_priorities");

    ASSERT_TRUE(critical.has_value());
    EXPECT_EQ(critical->first, "critical_task");
    ASSERT_TRUE(low.has_value());
    EXPECT_EQ(low->first, "low_priority_task");
}

// ============================================================================
// Main - Run all tests
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
