// infrastructure/serialization/serializer/core/log_deserializer.hpp
#ifndef SERIALIZER_CORE_LOG_DESERIALIZER_H_
#define SERIALIZER_CORE_LOG_DESERIALIZER_H_

#include <nlohmann/json.hpp>

#include "domain/model/daily_log.hpp"

namespace serializer::core {

class LogDeserializer {
 public:
  /**
   * @brief 将 JSON 对象反序列化为 DailyLog 结构体
   * @throws nlohmann::json::exception 解析错误时抛出
   */
  static auto Deserialize(const nlohmann::json& day_json) -> DailyLog;
};

}  // namespace serializer::core

#endif  // SERIALIZER_CORE_LOG_DESERIALIZER_H_
