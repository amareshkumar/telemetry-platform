#include "telemetry_common/config.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace telemetry_common {

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

bool Config::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line, current_section;
    while (std::getline(file, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Section header
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Key-value pair
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = trim(line.substr(0, pos));
            std::string value = trim(line.substr(pos + 1));
            data_[current_section][key] = value;
        }
    }
    
    return true;
}

std::string Config::get_string(const std::string& section, 
                               const std::string& key,
                               const std::string& default_value) const {
    auto sec_it = data_.find(section);
    if (sec_it == data_.end()) {
        return default_value;
    }
    
    auto key_it = sec_it->second.find(key);
    if (key_it == sec_it->second.end()) {
        return default_value;
    }
    
    return key_it->second;
}

int Config::get_int(const std::string& section,
                   const std::string& key,
                   int default_value) const {
    std::string value = get_string(section, key);
    if (value.empty()) {
        return default_value;
    }
    
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

bool Config::get_bool(const std::string& section,
                     const std::string& key,
                     bool default_value) const {
    std::string value = get_string(section, key);
    if (value.empty()) {
        return default_value;
    }
    
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return value == "true" || value == "1" || value == "yes" || value == "on";
}

double Config::get_double(const std::string& section,
                         const std::string& key,
                         double default_value) const {
    std::string value = get_string(section, key);
    if (value.empty()) {
        return default_value;
    }
    
    try {
        return std::stod(value);
    } catch (...) {
        return default_value;
    }
}

bool Config::has_key(const std::string& section, const std::string& key) const {
    auto sec_it = data_.find(section);
    if (sec_it == data_.end()) {
        return false;
    }
    return sec_it->second.find(key) != sec_it->second.end();
}

} // namespace telemetry_common
