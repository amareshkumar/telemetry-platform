# Core library
add_library(TELEMETRY_PROCESSOR_core
    core/Task.cpp
    core/RedisClient.cpp
    task_queue.cpp
)

target_include_directories(TELEMETRY_PROCESSOR_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(TELEMETRY_PROCESSOR_core
    PUBLIC
        nlohmann_json::nlohmann_json
)

# Executable (optional - for testing)
add_executable(TELEMETRY_PROCESSOR_demo
    main.cpp
)

target_link_libraries(TELEMETRY_PROCESSOR_demo
    PRIVATE
        TELEMETRY_PROCESSOR_core
)
