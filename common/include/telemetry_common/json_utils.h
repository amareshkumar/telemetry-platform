#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace telemetry_common {

/**
 * @brief Serialize JSON to string
 */
std::string json_to_string(const nlohmann::json& j, int indent = -1);

/**
 * @brief Parse JSON from string
 * @return JSON object or null if parsing fails
 */
nlohmann::json string_to_json(const std::string& str);

/**
 * @brief Validate JSON against simple schema
 * @param j JSON object to validate
 * @param required_fields List of required field names
 * @return true if all required fields present
 */
bool validate_json(const nlohmann::json& j, const std::vector<std::string>& required_fields);

/**
 * @brief Pretty print JSON
 */
std::string pretty_print_json(const nlohmann::json& j);

} // namespace telemetry_common
