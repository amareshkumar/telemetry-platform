# Google Test/Mock Quick Reference - Interview Cheat Sheet

## Google Test Assertions

### Basic Assertions
```cpp
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);
EXPECT_EQ(expected, actual);
EXPECT_NE(val1, val2);
EXPECT_LT(val1, val2);  // Less than
EXPECT_LE(val1, val2);  // Less or equal
EXPECT_GT(val1, val2);  // Greater than
EXPECT_GE(val1, val2);  // Greater or equal
```

### String Assertions
```cpp
EXPECT_STREQ("hello", str);   // C-strings equal
EXPECT_STRNE("hello", str);   // C-strings not equal
EXPECT_STRCASEEQ("HELLO", str);  // Case-insensitive
```

### Floating Point
```cpp
EXPECT_FLOAT_EQ(expected, actual);
EXPECT_DOUBLE_EQ(expected, actual);
EXPECT_NEAR(val1, val2, abs_error);
```

## Google Mock Expectations

### Basic Expectations
```cpp
EXPECT_CALL(mock_object, method(arg1, arg2))
    .WillOnce(Return(value));

EXPECT_CALL(mock_object, method(_))  // Any argument
    .WillRepeatedly(Return(value));

EXPECT_CALL(mock_object, method(Eq(42)))  // Matcher
    .Times(3)
    .WillOnce(Return(1))
    .WillOnce(Return(2))
    .WillOnce(Return(3));
```

### Argument Matchers
```cpp
EXPECT_CALL(mock, method(_));           // Any value
EXPECT_CALL(mock, method(Eq(42)));      // Equal to 42
EXPECT_CALL(mock, method(Ne(42)));      // Not equal
EXPECT_CALL(mock, method(Lt(10)));      // Less than
EXPECT_CALL(mock, method(Ge(5)));       // Greater or equal
EXPECT_CALL(mock, method(IsNull()));    // nullptr
EXPECT_CALL(mock, method(NotNull()));   // Not nullptr
```

### String Matchers
```cpp
EXPECT_CALL(mock, method(StrEq("hello")));
EXPECT_CALL(mock, method(StrNe("bye")));
EXPECT_CALL(mock, method(HasSubstr("world")));
EXPECT_CALL(mock, method(StartsWith("prefix")));
EXPECT_CALL(mock, method(EndsWith("suffix")));
EXPECT_CALL(mock, method(MatchesRegex(".*[0-9]+")));
```

### Container Matchers
```cpp
EXPECT_CALL(mock, method(IsEmpty()));
EXPECT_CALL(mock, method(SizeIs(3)));
EXPECT_CALL(mock, method(Contains(42)));
EXPECT_CALL(mock, method(ElementsAre(1, 2, 3)));
```

### Optional Matchers (C++17)
```cpp
EXPECT_THAT(opt, Optional(Eq(42)));      // has_value() && value == 42
EXPECT_THAT(opt, Eq(std::nullopt));      // !has_value()
```

## Test Fixtures

### Basic Fixture
```cpp
class MyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Runs before each test
        mock_ = std::make_unique<MockClass>();
    }
    
    void TearDown() override {
        // Runs after each test
        mock_.reset();
    }
    
    std::unique_ptr<MockClass> mock_;
};

TEST_F(MyTest, TestName) {
    // Use mock_ here
}
```

### Parameterized Tests
```cpp
class MyParamTest : public ::testing::TestWithParam<int> {};

TEST_P(MyParamTest, TestName) {
    int value = GetParam();
    EXPECT_GT(value, 0);
}

INSTANTIATE_TEST_SUITE_P(
    ValuesRange,
    MyParamTest,
    ::testing::Values(1, 2, 3, 5, 8)
);
```

## Actions

### Return Actions
```cpp
.WillOnce(Return(value))
.WillOnce(ReturnRef(ref))
.WillOnce(ReturnPointee(&ptr))
```

### Side Effects
```cpp
.WillOnce(DoAll(
    SetArgPointee<0>(value),  // Set *arg0 = value
    Return(true)
))
```

### Custom Actions
```cpp
.WillOnce(Invoke([](int x) { return x * 2; }))
.WillOnce(InvokeWithoutArgs([]() { return 42; }))
```

## Cardinality

```cpp
.Times(5)                    // Exactly 5 times
.Times(AtLeast(3))           // 3 or more
.Times(AtMost(10))           // 10 or fewer
.Times(Between(2, 5))        // 2 to 5 times
.Times(AnyNumber())          // Any number (including 0)
```

## Common Patterns

### Testing Optional Return
```cpp
TEST_F(RedisTest, GetReturnsValue) {
    EXPECT_CALL(*mock_redis_, get("key"))
        .WillOnce(Return(std::optional<std::string>("value")));
    
    auto result = mock_redis_->get("key");
    EXPECT_THAT(result, Optional(Eq("value")));
}
```

### Testing Empty Optional
```cpp
TEST_F(RedisTest, GetReturnsNullopt) {
    EXPECT_CALL(*mock_redis_, get("missing"))
        .WillOnce(Return(std::nullopt));
    
    auto result = mock_redis_->get("missing");
    EXPECT_FALSE(result.has_value());
}
```

### Testing Containers
```cpp
TEST_F(RedisTest, LrangeReturnsItems) {
    std::vector<std::string> items = {"a", "b", "c"};
    EXPECT_CALL(*mock_redis_, lrange("list", 0, -1))
        .WillOnce(Return(items));
    
    auto result = mock_redis_->lrange("list", 0, -1);
    EXPECT_THAT(result, SizeIs(3));
    EXPECT_THAT(result, ElementsAre("a", "b", "c"));
}
```

### Testing Sequences
```cpp
TEST_F(RedisTest, IncrementSequence) {
    EXPECT_CALL(*mock_redis_, incr("counter"))
        .WillOnce(Return(1))
        .WillOnce(Return(2))
        .WillOnce(Return(3));
    
    EXPECT_EQ(mock_redis_->incr("counter"), 1);
    EXPECT_EQ(mock_redis_->incr("counter"), 2);
    EXPECT_EQ(mock_redis_->incr("counter"), 3);
}
```

## Interview Q&A

### Q: "What's the difference between EXPECT and ASSERT?"
**A:** 
- `EXPECT_*`: Reports failure, continues test (check multiple conditions)
- `ASSERT_*`: Reports failure, stops test (prerequisite check)

### Q: "When do you use WillOnce vs WillRepeatedly?"
**A:**
- `WillOnce()`: Single call with specific return
- `WillRepeatedly()`: All future calls return same value
- Use `WillOnce()` for most tests (explicit, predictable)

### Q: "How do you test void functions?"
**A:**
```cpp
// Use DoAll for side effects
EXPECT_CALL(mock, voidMethod())
    .WillOnce(DoAll(
        SetArgPointee<0>(42),  // Modify output parameter
        InvokeWithoutArgs([]() { /* side effect */ })
    ));
```

### Q: "How do matchers improve tests?"
**A:**
- **Composable**: `Optional(Eq(42))` reads like English
- **Better errors**: "Expected: optional(42), Actual: nullopt"
- **Type-safe**: Compile-time checking

### Q: "What's a test fixture?"
**A:** Class that provides:
- **Setup**: Initialize mocks/resources before each test
- **Teardown**: Cleanup after each test
- **Shared state**: Members accessible in all tests
- **DRY**: Write setup once, reuse in all tests

## Pro Tips

1. **Use matchers over raw comparisons**
   ```cpp
   // ❌ Bad
   EXPECT_EQ(result.has_value(), true);
   EXPECT_EQ(*result, "value");
   
   // ✅ Good
   EXPECT_THAT(result, Optional(Eq("value")));
   ```

2. **EXPECT_CALL before action**
   ```cpp
   // ✅ Always set expectation first
   EXPECT_CALL(mock, method()).WillOnce(Return(42));
   auto result = mock.method();  // Then call
   ```

3. **Use test fixtures for common setup**
   ```cpp
   // ✅ Setup once in fixture
   class MyTest : public ::testing::Test {
   protected:
       void SetUp() override { mock_ = std::make_unique<Mock>(); }
       std::unique_ptr<Mock> mock_;
   };
   ```

4. **Test one thing per test**
   ```cpp
   // ✅ Focused test
   TEST_F(RedisTest, GetReturnsValue) { /* ... */ }
   TEST_F(RedisTest, GetReturnsNullopt) { /* ... */ }
   ```

---

**Print this for interviews! 🚀**
