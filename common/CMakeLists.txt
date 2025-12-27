cmake_minimum_required(VERSION 3.14)

project(TelemetryCommon VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common library sources
set(COMMON_SOURCES
    src/redis_client.cpp
    src/json_utils.cpp
    src/config.cpp
    src/uuid_generator.cpp
    src/proto_adapter.cpp
)

# Common library headers
set(COMMON_HEADERS
    include/telemetry_common/redis_client.h
    include/telemetry_common/json_utils.h
    include/telemetry_common/config.h
    include/telemetry_common/uuid_generator.h
    include/telemetry_common/types.h
    include/telemetry_common/proto_adapter.h
)

# Create common library
add_library(telemetry_common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

# Include directories
target_include_directories(telemetry_common
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Fetch dependencies
include(FetchContent)

# nlohmann/json (used by both projects)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)

# Hiredis (required by redis-plus-plus) - using master for modern CMake
FetchContent_Declare(
    hiredis
    GIT_REPOSITORY https://github.com/redis/hiredis.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
set(DISABLE_TESTS ON CACHE BOOL "Disable hiredis tests")
set(ENABLE_SSL OFF CACHE BOOL "Disable SSL")
FetchContent_MakeAvailable(hiredis)

# Copy hiredis headers to a subdirectory so redis++ can find it as hiredis/hiredis.h
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper/hiredis")
file(GLOB HIREDIS_HEADERS "${hiredis_SOURCE_DIR}/*.h")
file(COPY ${HIREDIS_HEADERS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper/hiredis")

# redis-plus-plus (modern C++ Redis client)
# Using master branch for CMake 3.14+ compatibility
FetchContent_Declare(
    redis++
    GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
# Configure redis-plus-plus options
set(REDIS_PLUS_PLUS_BUILD_TEST OFF CACHE BOOL "Disable redis++ tests")
set(REDIS_PLUS_PLUS_BUILD_STATIC ON CACHE BOOL "Build static library")
set(REDIS_PLUS_PLUS_BUILD_SHARED OFF CACHE BOOL "Disable shared library")
# Point redis++ to hiredis wrapper so it can find hiredis/hiredis.h
set(HIREDIS_HEADER "${CMAKE_CURRENT_BINARY_DIR}/hiredis_wrapper" CACHE PATH "Path to hiredis headers")
set(HIREDIS_LIB hiredis CACHE STRING "Hiredis library target")
FetchContent_MakeAvailable(redis++)
FetchContent_MakeAvailable(redis++)

# Google Protobuf (for efficient binary serialization)
FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v25.1
    GIT_SHALLOW TRUE
)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "Disable protobuf tests")
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "Build static library")
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "Use dynamic MSVC runtime")
set(protobuf_WITH_ZLIB OFF CACHE BOOL "Disable ZLIB dependency")
FetchContent_MakeAvailable(protobuf)

# Generate .proto files using protobuf_generate
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")

# Create proto library with generated sources
add_library(telemetry_proto STATIC ${PROTO_FILES})
target_link_libraries(telemetry_proto PUBLIC protobuf::libprotobuf)
target_include_directories(telemetry_proto PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

# Use protobuf_generate to create .pb.cc and .pb.h files
# Explicitly include the protobuf-generate module
include("${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake")
protobuf_generate(
    TARGET telemetry_proto
    LANGUAGE cpp
    PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

# Link dependencies
target_link_libraries(telemetry_common 
    PUBLIC 
        nlohmann_json::nlohmann_json
        redis++::redis++_static
        telemetry_proto
)

# Compiler warnings
if(MSVC)
    target_compile_options(telemetry_common PRIVATE /W4)
else()
    target_compile_options(telemetry_common PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Build tests
if(BUILD_TESTS)
    # Fetch GoogleTest
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    # Integration test - requires real Redis server
    add_executable(test_redis_connection tests/test_redis_connection.cpp)
    target_link_libraries(test_redis_connection PRIVATE telemetry_common)
    
    # Unit test - uses Google Mock (no server required)
    add_executable(test_redis_client_unit tests/test_redis_client_unit.cpp)
    target_include_directories(test_redis_client_unit PRIVATE 
        tests
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(test_redis_client_unit PRIVATE 
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
    )
    
    # Protobuf adapter unit test
    add_executable(test_proto_adapter tests/test_proto_adapter.cpp)
    target_include_directories(test_proto_adapter PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}  # For generated .pb.h
    )
    target_link_libraries(test_proto_adapter PRIVATE 
        telemetry_common
        GTest::gtest
        GTest::gtest_main
    )
    
    # Add as CTest tests
    enable_testing()
    add_test(NAME redis_unit_tests COMMAND test_redis_client_unit)
    add_test(NAME proto_adapter_tests COMMAND test_proto_adapter)
    # add_test(NAME redis_integration_test COMMAND test_redis_connection)  # Requires Redis running
endif()

# Installation rules
install(TARGETS telemetry_common
    EXPORT TelemetryCommonTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/telemetry_common
    DESTINATION include
)
