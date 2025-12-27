#pragma once

#include "telemetry.pb.h"
#include <chrono>
#include <optional>
#include <string>

namespace telemetry {

// C++ struct for telemetry data (non-Protobuf)
struct TelemetrySampleCpp {
    std::chrono::system_clock::time_point timestamp{};
    double value = 0.0;
    std::string unit{"unitless"};
    uint32_t sequence_id = 0;
};

// Adapter for converting between C++ structs and Protobuf messages
// Provides type-safe serialization for telemetry data
class ProtoAdapter {
public:
    // Type aliases for clarity
    using TelemetrySampleProto = telemetry::TelemetrySample;  // From telemetry.pb.h
    using CppSample = TelemetrySampleCpp;                      // C++ struct above

    // Convert C++ struct to Protobuf message
    static TelemetrySampleProto toProto(const CppSample& sample);
    
    // Convert Protobuf message to C++ struct
    static CppSample fromProto(const TelemetrySampleProto& proto);
    
    // Serialize C++ struct to binary string
    static std::string serialize(const CppSample& sample);
    
    // Deserialize binary string to C++ struct
    static std::optional<CppSample> deserialize(const std::string& binary_data);
    
    // Get size of serialized message (for performance comparison)
    static size_t serializedSize(const CppSample& sample);
};

} // namespace telemetry
