#include <gtest/gtest.h>
#include "telemetry_processor/RedisClient.h"

using namespace telemetry_processor;

TEST(RedisClientTest, Connect) {
    RedisClient client;
    EXPECT_TRUE(client.connect());
    EXPECT_TRUE(client.is_connected());
}

TEST(RedisClientTest, Ping) {
    RedisClient client;
    client.connect();
    
    auto response = client.ping();
    EXPECT_EQ(response, "PONG");
}

TEST(RedisClientTest, SetAndGet) {
    RedisClient client;
    client.connect();
    
    EXPECT_TRUE(client.set("test_key", "test_value"));
    
    auto value = client.get("test_key");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "test_value");
}

TEST(RedisClientTest, GetNonExistent) {
    RedisClient client;
    client.connect();
    
    auto value = client.get("nonexistent_key");
    EXPECT_FALSE(value.has_value());
}

TEST(RedisClientTest, Delete) {
    RedisClient client;
    client.connect();
    
    client.set("delete_test", "value");
    EXPECT_TRUE(client.del("delete_test"));
    
    auto value = client.get("delete_test");
    EXPECT_FALSE(value.has_value());
}

TEST(RedisClientTest, RPushAndBLPop) {
    RedisClient client;
    client.connect();
    
    std::string queue = "test_queue";
    
    EXPECT_TRUE(client.rpush(queue, "item1"));
    EXPECT_TRUE(client.rpush(queue, "item2"));
    EXPECT_TRUE(client.rpush(queue, "item3"));
    
    EXPECT_EQ(client.llen(queue), 3);
    
    auto item1 = client.blpop(queue);
    ASSERT_TRUE(item1.has_value());
    EXPECT_EQ(*item1, "item1");
    
    auto item2 = client.blpop(queue);
    ASSERT_TRUE(item2.has_value());
    EXPECT_EQ(*item2, "item2");
    
    EXPECT_EQ(client.llen(queue), 1);
}

TEST(RedisClientTest, BLPopEmptyQueue) {
    RedisClient client;
    client.connect();
    
    auto item = client.blpop("empty_queue");
    EXPECT_FALSE(item.has_value());
}

TEST(RedisClientTest, QueueLength) {
    RedisClient client;
    client.connect();
    
    std::string queue = "length_test";
    
    EXPECT_EQ(client.llen(queue), 0);
    
    client.rpush(queue, "a");
    client.rpush(queue, "b");
    EXPECT_EQ(client.llen(queue), 2);
    
    client.blpop(queue);
    EXPECT_EQ(client.llen(queue), 1);
}

TEST(RedisClientTest, OperationsWithoutConnect) {
    RedisClient client;
    // Not calling connect()
    
    EXPECT_FALSE(client.is_connected());
    EXPECT_EQ(client.ping(), "");
    EXPECT_FALSE(client.set("key", "value"));
    EXPECT_FALSE(client.get("key").has_value());
    EXPECT_FALSE(client.rpush("queue", "item"));
    EXPECT_FALSE(client.blpop("queue").has_value());
}
