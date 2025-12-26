cmake_minimum_required(VERSION 3.20)

project(TelemetryPlatform 
    VERSION 1.0.0
    DESCRIPTION "Integrated Telemetry Platform: Ingestion + Processing"
    LANGUAGES CXX
)

# C++17 standard for consistency
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "=================================================")
message(STATUS "Telemetry Platform - Monorepo Build")
message(STATUS "=================================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "")

# Options
option(BUILD_INGESTION "Build TelemetryHub ingestion service" ON)
option(BUILD_PROCESSING "Build TelemetryTaskProcessor processing service" ON)
option(BUILD_COMMON "Build common shared library" ON)
option(BUILD_TESTS "Build all tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Build order: common first (dependency for both projects)
if(BUILD_COMMON)
    message(STATUS "Building: common/ (shared library)")
    add_subdirectory(common)
endif()

# Build ingestion service (TelemetryHub)
if(BUILD_INGESTION)
    message(STATUS "Building: ingestion/ (TelemetryHub)")
    add_subdirectory(ingestion)
endif()

# Build processing service (TelemetryTaskProcessor)
if(BUILD_PROCESSING)
    message(STATUS "Building: processing/ (TelemetryTaskProcessor)")
    add_subdirectory(processing)
endif()

# Integration tests (optional)
if(BUILD_TESTS)
    enable_testing()
    message(STATUS "Building: tests/integration/")
    # add_subdirectory(tests/integration)  # TODO: Create integration tests
endif()

message(STATUS "")
message(STATUS "=================================================")
message(STATUS "Configuration Summary:")
message(STATUS "  Common Library:       ${BUILD_COMMON}")
message(STATUS "  Ingestion Service:    ${BUILD_INGESTION}")
message(STATUS "  Processing Service:   ${BUILD_PROCESSING}")
message(STATUS "  Tests:                ${BUILD_TESTS}")
message(STATUS "  Examples:             ${BUILD_EXAMPLES}")
message(STATUS "=================================================")
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  cmake --build build --target telemetry_common      (common library)")
message(STATUS "  cmake --build build --target telemetry_gateway     (ingestion service)")
message(STATUS "  cmake --build build --target telemetry_processor   (processing service)")
message(STATUS "  cmake --build build                                (build all)")
message(STATUS "=================================================")
