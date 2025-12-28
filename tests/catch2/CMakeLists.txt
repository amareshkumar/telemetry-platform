# Catch2 BDD-Style Tests
# Modern C++ testing with SCENARIO/GIVEN/WHEN/THEN syntax

cmake_minimum_required(VERSION 3.14)

# Executable for Catch2 tests
add_executable(catch2_tests
    test_task_queue_catch2.cpp
)

# Link against Catch2::Catch2WithMain (includes main() function)
target_link_libraries(catch2_tests PRIVATE
    Catch2::Catch2WithMain
    TELEMETRY_PROCESSOR_core  # TaskQueue implementation
)

# Include directories
target_include_directories(catch2_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/processing/include
)

# Register with CTest
include(CTest)
include(Catch)
catch_discover_tests(catch2_tests
    TEST_PREFIX "Catch2."
    REPORTER junit
    OUTPUT_DIR ${CMAKE_BINARY_DIR}/test_results
)

message(STATUS "  Catch2 test target: catch2_tests")
message(STATUS "  Run: ./catch2_tests --reporter compact")
message(STATUS "  Run with tags: ./catch2_tests \"[priority]\"")
