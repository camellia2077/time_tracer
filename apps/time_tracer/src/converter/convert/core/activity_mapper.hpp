// converter/convert/core/activity_mapper.hpp
#ifndef CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_
#define CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_

#include <string>
#include <vector>

#include "common/config/models/converter_config_models.hpp"
#include "domain/model/daily_log.hpp"

class ActivityMapper {
 public:
  explicit ActivityMapper(const ConverterConfig& config);
  void map_activities(DailyLog& day);

 private:
  const ConverterConfig& config_;
  // 保持引用以避免拷贝，或者拷贝 vector (取决于数据量，引用通常更高效)
  const std::vector<std::string>& wake_keywords_;

  static std::string formatTime(const std::string& timeStrHHMM);
  static int calculateDurationMinutes(const std::string& startTimeStr,
                                      const std::string& endTimeStr);
};

#endif  // CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_
