// serializer/core/log_serializer.hpp
#ifndef SERIALIZER_CORE_LOG_SERIALIZER_H_
#define SERIALIZER_CORE_LOG_SERIALIZER_H_

#include <nlohmann/json.hpp>

#include "domain/model/daily_log.hpp"

namespace serializer::core {

class LogSerializer {
 public:
  /**
   * @brief 将单个 DailyLog 结构体序列化为 JSON 对象
   */
  static nlohmann::json serialize(const DailyLog& day);
};

}  // namespace serializer::core

#endif  // SERIALIZER_CORE_LOG_SERIALIZER_H_
