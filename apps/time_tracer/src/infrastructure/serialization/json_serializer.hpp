// infrastructure/serialization/serializer/json_serializer.hpp
#ifndef SERIALIZER_JSON_SERIALIZER_H_
#define SERIALIZER_JSON_SERIALIZER_H_

#include <nlohmann/json.hpp>
#include <vector>

#include "domain/model/daily_log.hpp"

namespace serializer {

/**
 * @brief 序列化模块的外观类 (Facade)
 * 负责调度内部核心组件 (LogSerializer, LogDeserializer) 完成转换任务。
 */
class JsonSerializer {
 public:
  // Struct -> JSON
  [[nodiscard]] static auto SerializeDay(const DailyLog& day) -> nlohmann::json;
  [[nodiscard]] static auto SerializeDays(const std::vector<DailyLog>& days)
      -> nlohmann::json;

  // JSON -> Struct
  [[nodiscard]] static auto DeserializeDay(const nlohmann::json& day_json)
      -> DailyLog;
  [[nodiscard]] static auto DeserializeDays(const nlohmann::json& json_array)
      -> std::vector<DailyLog>;
};

}  // namespace serializer

#endif  // SERIALIZER_JSON_SERIALIZER_H_
