# Simple producer example
add_executable(telemetry_simple_producer
    telemetry_simple_producer.cpp
)

target_link_libraries(telemetry_simple_producer
    PRIVATE
        TELEMETRY_PROCESSOR_core
)
