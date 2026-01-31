// serializer/json_serializer.hpp
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
  static nlohmann::json serializeDay(const DailyLog& day);
  static nlohmann::json serializeDays(const std::vector<DailyLog>& days);

  // JSON -> Struct
  static DailyLog deserializeDay(const nlohmann::json& day_json);
  static std::vector<DailyLog> deserializeDays(
      const nlohmann::json& json_array);
};

}  // namespace serializer

#endif  // SERIALIZER_JSON_SERIALIZER_H_
