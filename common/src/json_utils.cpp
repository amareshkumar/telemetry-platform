#include "telemetry_common/json_utils.h"
#include <stdexcept>

namespace telemetry_common {

std::string json_to_string(const nlohmann::json& j, int indent) {
    return j.dump(indent);
}

nlohmann::json string_to_json(const std::string& str) {
    try {
        return nlohmann::json::parse(str);
    } catch (const nlohmann::json::exception& e) {
        return nlohmann::json();  // Return empty JSON on error
    }
}

bool validate_json(const nlohmann::json& j, const std::vector<std::string>& required_fields) {
    for (const auto& field : required_fields) {
        if (!j.contains(field)) {
            return false;
        }
    }
    return true;
}

std::string pretty_print_json(const nlohmann::json& j) {
    return j.dump(2);  // 2-space indentation
}

} // namespace telemetry_common
