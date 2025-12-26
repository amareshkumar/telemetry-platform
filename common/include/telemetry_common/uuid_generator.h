#pragma once

#include <string>

namespace telemetry_common {

/**
 * @brief Generate UUID v4 (random)
 * @return UUID string in format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
 */
std::string generate_uuid();

} // namespace telemetry_common
