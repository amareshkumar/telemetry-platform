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

# Fetch nlohmann/json (used by both projects)
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)

# Link nlohmann/json
target_link_libraries(telemetry_common PUBLIC nlohmann_json::nlohmann_json)

# Compiler warnings
if(MSVC)
    target_compile_options(telemetry_common PRIVATE /W4)
else()
    target_compile_options(telemetry_common PRIVATE -Wall -Wextra -Wpedantic)
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
