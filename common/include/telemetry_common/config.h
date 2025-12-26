#pragma once

#include <string>
#include <map>

namespace telemetry_common {

/**
 * @brief Simple INI-style configuration parser
 * 
 * Supports format:
 * [section]
 * key = value
 */
class Config {
public:
    /**
     * @brief Load configuration from file
     */
    bool load_from_file(const std::string& filename);
    
    /**
     * @brief Get string value
     * @param section Section name
     * @param key Key name
     * @param default_value Default if not found
     */
    std::string get_string(const std::string& section, 
                          const std::string& key,
                          const std::string& default_value = "") const;
    
    /**
     * @brief Get integer value
     */
    int get_int(const std::string& section,
               const std::string& key,
               int default_value = 0) const;
    
    /**
     * @brief Get boolean value
     */
    bool get_bool(const std::string& section,
                 const std::string& key,
                 bool default_value = false) const;
    
    /**
     * @brief Get double value
     */
    double get_double(const std::string& section,
                     const std::string& key,
                     double default_value = 0.0) const;
    
    /**
     * @brief Check if key exists
     */
    bool has_key(const std::string& section, const std::string& key) const;

private:
    std::map<std::string, std::map<std::string, std::string>> data_;
};

} // namespace telemetry_common
