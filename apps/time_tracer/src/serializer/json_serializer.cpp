// serializer/json_serializer.cpp
#include "json_serializer.hpp"
#include "serializer/core/log_deserializer.hpp"
#include "serializer/core/log_serializer.hpp"
#include <iostream>

namespace serializer {

// 使用 core 命名空间简化调用
using core::LogSerializer;
using core::LogDeserializer;

// --- Serialization Facade ---

nlohmann::json JsonSerializer::serializeDay(const DailyLog& day) {
    return LogSerializer::serialize(day);
}

nlohmann::json JsonSerializer::serializeDays(const std::vector<DailyLog>& days) {
    nlohmann::json j_array = nlohmann::json::array();
    for (const auto& day : days) {
        if (!day.date.empty()) {
            j_array.push_back(LogSerializer::serialize(day));
        }
    }
    return j_array;
}

// --- Deserialization Facade ---

DailyLog JsonSerializer::deserializeDay(const nlohmann::json& day_json) {
    return LogDeserializer::deserialize(day_json);
}

std::vector<DailyLog> JsonSerializer::deserializeDays(const nlohmann::json& json_array) {
    std::vector<DailyLog> days;
    if (!json_array.is_array()) {
        return days;
    }
    for (const auto& j : json_array) {
        days.push_back(LogDeserializer::deserialize(j));
    }
    return days;
}

} // namespace serializer
