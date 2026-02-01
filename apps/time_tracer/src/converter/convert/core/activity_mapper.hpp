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

  bool is_wake_event(const RawEvent& raw_event) const;
  std::string map_description(const std::string& description) const;
  std::string apply_duration_rules(const std::string& mapped_description,
                                   const std::string& start_time,
                                   const std::string& end_time) const;
  void apply_top_parent_mapping(std::vector<std::string>& parts) const;
  static std::string build_project_path(const std::vector<std::string>& parts);
  void append_activity(DailyLog& day, const RawEvent& raw_event,
                       const std::string& start_time,
                       const std::string& end_time,
                       const std::string& mapped_description) const;

  static std::string formatTime(const std::string& time_str_hhmm);
  static int calculateDurationMinutes(const std::string& start_time_str,
                                      const std::string& end_time_str);
};

#endif  // CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_
