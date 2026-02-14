// infrastructure/serialization/core/log_codec.hpp
#ifndef INFRASTRUCTURE_SERIALIZATION_CORE_LOG_CODEC_H_
#define INFRASTRUCTURE_SERIALIZATION_CORE_LOG_CODEC_H_

#include <nlohmann/json.hpp>

#include "domain/model/daily_log.hpp"

namespace serializer::core {

class LogSerializer {
 public:
  static auto Serialize(const DailyLog& day) -> nlohmann::json;
};

class LogDeserializer {
 public:
  static auto Deserialize(const nlohmann::json& day_json) -> DailyLog;
};

}  // namespace serializer::core

#endif  // INFRASTRUCTURE_SERIALIZATION_CORE_LOG_CODEC_H_
