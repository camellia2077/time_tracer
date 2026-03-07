// infrastructure/serialization/json_serializer.hpp
#ifndef INFRASTRUCTURE_SERIALIZATION_JSON_SERIALIZER_H_
#define INFRASTRUCTURE_SERIALIZATION_JSON_SERIALIZER_H_

#include <string>
#include <string_view>
#include <vector>

#include "domain/model/daily_log.hpp"

namespace serializer {

class JsonSerializer {
 public:
  [[nodiscard]] static auto SerializeDay(const DailyLog& day, int indent = -1)
      -> std::string;
  [[nodiscard]] static auto SerializeDays(const std::vector<DailyLog>& days)
      -> std::string;
  [[nodiscard]] static auto SerializeDays(const std::vector<DailyLog>& days,
                                          int indent) -> std::string;

  [[nodiscard]] static auto DeserializeDay(std::string_view day_json)
      -> DailyLog;
  [[nodiscard]] static auto DeserializeDays(std::string_view json_array)
      -> std::vector<DailyLog>;
};

}  // namespace serializer

#endif  // INFRASTRUCTURE_SERIALIZATION_JSON_SERIALIZER_H_
