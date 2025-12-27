#include "telemetry_common/proto_adapter.h"
#include <chrono>

namespace telemetry {

ProtoAdapter::TelemetrySampleProto ProtoAdapter::toProto(const CppSample& sample)
{
    TelemetrySampleProto proto;
    
    // Convert timestamp to microseconds since epoch
    auto timestamp_us = std::chrono::duration_cast<std::chrono::microseconds>(
        sample.timestamp.time_since_epoch()).count();
    proto.set_timestamp_us(timestamp_us);
    
    proto.set_value(sample.value);
    proto.set_unit(sample.unit);
    proto.set_sequence_id(sample.sequence_id);
    
    return proto;
}

ProtoAdapter::CppSample ProtoAdapter::fromProto(const TelemetrySampleProto& proto)
{
    CppSample sample;
    
    // Convert microseconds back to time_point
    auto timestamp_us = std::chrono::microseconds(proto.timestamp_us());
    sample.timestamp = std::chrono::system_clock::time_point(timestamp_us);
    
    sample.value = proto.value();
    sample.unit = proto.unit();
    sample.sequence_id = proto.sequence_id();
    
    return sample;
}

std::string ProtoAdapter::serialize(const CppSample& sample)
{
    auto proto = toProto(sample);
    std::string binary_data;
    proto.SerializeToString(&binary_data);
    return binary_data;
}

std::optional<ProtoAdapter::CppSample> ProtoAdapter::deserialize(const std::string& binary_data)
{
    TelemetrySampleProto proto;
    if (!proto.ParseFromString(binary_data)) {
        return std::nullopt;  // Parse failed
    }
    return fromProto(proto);
}

size_t ProtoAdapter::serializedSize(const CppSample& sample)
{
    auto proto = toProto(sample);
    return proto.ByteSizeLong();
}

} // namespace telemetry
