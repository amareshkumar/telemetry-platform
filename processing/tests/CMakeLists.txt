# Unit tests
add_executable(TELEMETRY_PROCESSOR_tests
    test_task.cpp
    test_redis_client.cpp
)

target_link_libraries(TELEMETRY_PROCESSOR_tests
    PRIVATE
        TELEMETRY_PROCESSOR_core
        GTest::gtest_main
)

# Discover tests
gtest_discover_tests(TELEMETRY_PROCESSOR_tests)
