#include "telemetry_common/proto_adapter.h"
#include <gtest/gtest.h>
#include <chrono>
#include <sstream>

using namespace telemetry;

// Test fixture for ProtoAdapter tests
class ProtoAdapterTest : public ::testing::Test {
protected:
    TelemetrySampleCpp createSample() {
        TelemetrySampleCpp sample;
        sample.timestamp = std::chrono::system_clock::now();
        sample.value = 23.5;
        sample.unit = "celsius";
        sample.sequence_id = 12345;
        return sample;
    }
};

// Basic round-trip conversion
TEST_F(ProtoAdapterTest, RoundTripConversion) {
    auto original = createSample();
    
    // Convert to proto and back
    auto proto = ProtoAdapter::toProto(original);
    auto restored = ProtoAdapter::fromProto(proto);
    
    // Verify all fields
    EXPECT_EQ(restored.value, 23.5);
    EXPECT_EQ(restored.unit, "celsius");
    EXPECT_EQ(restored.sequence_id, 12345);
    
    // Timestamp should be within 1 microsecond (precision loss acceptable)
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(
        original.timestamp - restored.timestamp).count();
    EXPECT_LE(std::abs(diff), 1);
}

// Serialize to binary and deserialize
TEST_F(ProtoAdapterTest, SerializeDeserialize) {
    auto original = createSample();
    
    // Serialize to binary
    auto binary_data = ProtoAdapter::serialize(original);
    
    // Binary should be smaller than JSON equivalent (~60 bytes)
    EXPECT_LT(binary_data.size(), 40);  // Protobuf should be <40 bytes
    
    // Deserialize
    auto restored = ProtoAdapter::deserialize(binary_data);
    ASSERT_TRUE(restored.has_value());
    
    EXPECT_EQ(restored->value, 23.5);
    EXPECT_EQ(restored->unit, "celsius");
    EXPECT_EQ(restored->sequence_id, 12345);
}

// Test with empty/default values
TEST_F(ProtoAdapterTest, DefaultValues) {
    TelemetrySampleCpp sample;
    
    auto binary_data = ProtoAdapter::serialize(sample);
    auto restored = ProtoAdapter::deserialize(binary_data);
    
    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->value, 0.0);
    EXPECT_EQ(restored->unit, "unitless");
    EXPECT_EQ(restored->sequence_id, 0);
}

// Test serialized size calculation
TEST_F(ProtoAdapterTest, SerializedSize) {
    auto sample = createSample();
    
    auto calculated_size = ProtoAdapter::serializedSize(sample);
    auto binary_data = ProtoAdapter::serialize(sample);
    
    EXPECT_EQ(calculated_size, binary_data.size());
    EXPECT_LT(calculated_size, 40);  // Protobuf efficiency
}

// Test with various units
TEST_F(ProtoAdapterTest, DifferentUnits) {
    std::vector<std::string> units = {"celsius", "fahrenheit", "pascal", "psi", "volts", "amps"};
    
    for (const auto& unit : units) {
        TelemetrySampleCpp sample;
        sample.value = 100.0;
        sample.unit = unit;
        sample.sequence_id = 1;
        
        auto binary_data = ProtoAdapter::serialize(sample);
        auto restored = ProtoAdapter::deserialize(binary_data);
        
        ASSERT_TRUE(restored.has_value());
        EXPECT_EQ(restored->unit, unit);
    }
}

// Test large sequence IDs
TEST_F(ProtoAdapterTest, LargeSequenceId) {
    TelemetrySampleCpp sample;
    sample.sequence_id = 4294967295;  // max uint32_t
    sample.value = 42.0;
    
    auto binary_data = ProtoAdapter::serialize(sample);
    auto restored = ProtoAdapter::deserialize(binary_data);
    
    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->sequence_id, 4294967295);
}

// Test extreme values
TEST_F(ProtoAdapterTest, ExtremeValues) {
    TelemetrySampleCpp sample;
    sample.value = 1e308;  // Near max double
    sample.unit = "extreme";
    
    auto binary_data = ProtoAdapter::serialize(sample);
    auto restored = ProtoAdapter::deserialize(binary_data);
    
    ASSERT_TRUE(restored.has_value());
    EXPECT_DOUBLE_EQ(restored->value, 1e308);
}

// Test invalid binary data
TEST_F(ProtoAdapterTest, InvalidBinaryData) {
    std::string garbage = "not a valid protobuf message";
    
    auto result = ProtoAdapter::deserialize(garbage);
    EXPECT_FALSE(result.has_value());
}

// Performance: Protobuf vs JSON comparison (informational)
TEST_F(ProtoAdapterTest, PerformanceComparison) {
    auto sample = createSample();
    
    // Protobuf serialization
    auto start_proto = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        auto binary = ProtoAdapter::serialize(sample);
    }
    auto end_proto = std::chrono::high_resolution_clock::now();
    auto proto_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_proto - start_proto).count();
    
    // JSON serialization (manual string construction - simulating current code)
    auto start_json = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        std::ostringstream json;
        json << "{\"seq\":" << sample.sequence_id
             << ",\"value\":" << sample.value
             << ",\"unit\":\"" << sample.unit << "\"}";
        auto _ = json.str();
    }
    auto end_json = std::chrono::high_resolution_clock::now();
    auto json_duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end_json - start_json).count();
    
    // Report performance (not an assertion, just informational)
    std::cout << "\n=== Performance Comparison (10k iterations) ===\n";
    std::cout << "Protobuf: " << proto_duration << " μs\n";
    std::cout << "JSON:     " << json_duration << " μs\n";
    std::cout << "Speedup:  " << (static_cast<double>(json_duration) / proto_duration) << "x\n";
    
    // Protobuf should be faster (typically 2-5x)
    EXPECT_LT(proto_duration, json_duration);
}

// Size comparison: Protobuf vs JSON
TEST_F(ProtoAdapterTest, SizeComparison) {
    auto sample = createSample();
    
    // Protobuf size
    auto proto_size = ProtoAdapter::serialize(sample).size();
    
    // JSON size (manual construction)
    std::ostringstream json;
    json << "{\"seq\":" << sample.sequence_id
         << ",\"value\":" << sample.value
         << ",\"unit\":\"" << sample.unit << "\"}";
    auto json_size = json.str().size();
    
    std::cout << "\n=== Size Comparison ===\n";
    std::cout << "Protobuf: " << proto_size << " bytes\n";
    std::cout << "JSON:     " << json_size << " bytes\n";
    std::cout << "Reduction: " << ((json_size - proto_size) * 100.0 / json_size) << "%\n";
    
    // Protobuf should be significantly smaller
    EXPECT_LT(proto_size, json_size);
    EXPECT_LT(proto_size, 40);   // Protobuf <40 bytes
    EXPECT_GT(json_size, 50);    // JSON >50 bytes
}
