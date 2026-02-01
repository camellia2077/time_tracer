// serializer/json_serializer.cpp
#include "json_serializer.hpp"

#include <iostream>

#include "serializer/core/log_deserializer.hpp"
#include "serializer/core/log_serializer.hpp"

namespace serializer {

// 使用 core 命名空间简化调用
using core::LogDeserializer;
using core::LogSerializer;

// --- Serialization Facade ---

auto JsonSerializer::serializeDay(const DailyLog& day) -> nlohmann::json {
  return LogSerializer::serialize(day);
}

auto JsonSerializer::serializeDays(const std::vector<DailyLog>& days)
    -> nlohmann::json {
  nlohmann::json j_array = nlohmann::json::array();
  for (const auto& day : days) {
    if (!day.date.empty()) {
      j_array.push_back(LogSerializer::serialize(day));
    }
  }
  return j_array;
}

// --- Deserialization Facade ---

auto JsonSerializer::deserializeDay(const nlohmann::json& day_json)
    -> DailyLog {
  return LogDeserializer::deserialize(day_json);
}

auto JsonSerializer::deserializeDays(const nlohmann::json& json_array)
    -> std::vector<DailyLog> {
  std::vector<DailyLog> days;
  if (!json_array.is_array()) {
    return days;
  }
  for (const auto& json_item : json_array) {
    days.push_back(LogDeserializer::deserialize(json_item));
  }
  return days;
}

}  // namespace serializer
