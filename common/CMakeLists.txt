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
)

# Common library headers
set(COMMON_HEADERS
    include/telemetry_common/redis_client.h
    include/telemetry_common/json_utils.h
    include/telemetry_common/config.h
    include/telemetry_common/uuid_generator.h
    include/telemetry_common/types.h
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

# redis-plus-plus (modern C++ Redis client)
FetchContent_Declare(
    redis++
    GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
    GIT_TAG 1.3.12
    GIT_SHALLOW TRUE
)
# Configure redis-plus-plus options
set(REDIS_PLUS_PLUS_BUILD_TEST OFF CACHE BOOL "Disable redis++ tests")
set(REDIS_PLUS_PLUS_BUILD_STATIC ON CACHE BOOL "Build static library")
FetchContent_MakeAvailable(redis++)

# Link dependencies
target_link_libraries(telemetry_common 
    PUBLIC 
        nlohmann_json::nlohmann_json
        redis++::redis++_static
)

# Compiler warnings
if(MSVC)
    target_compile_options(telemetry_common PRIVATE /W4)
else()
    target_compile_options(telemetry_common PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Build tests
if(BUILD_TESTS)
    add_executable(test_redis_connection tests/test_redis_connection.cpp)
    target_link_libraries(test_redis_connection PRIVATE telemetry_common)
    
    # Add as CTest test (optional - requires Redis running)
    # enable_testing()
    # add_test(NAME redis_connection_test COMMAND test_redis_connection)
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
