#pragma once

#include <string>
#include <chrono>
#include <cstdint>

namespace telemetry_common {

/**
 * @brief Common types used across telemetry platform
 */

// Type aliases
using timestamp_t = std::chrono::system_clock::time_point;
using device_id_t = std::string;
using task_id_t = std::string;

/**
 * @brief Get current timestamp
 */
inline timestamp_t now() {
    return std::chrono::system_clock::now();
}

/**
 * @brief Convert timestamp to ISO8601 string
 */
std::string timestamp_to_iso8601(timestamp_t ts);

/**
 * @brief Parse ISO8601 string to timestamp
 */
timestamp_t iso8601_to_timestamp(const std::string& iso_str);

/**
 * @brief Get timestamp in milliseconds since epoch
 */
int64_t timestamp_to_ms(timestamp_t ts);

} // namespace telemetry_common
