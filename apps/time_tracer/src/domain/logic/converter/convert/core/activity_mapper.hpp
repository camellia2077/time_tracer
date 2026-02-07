// domain/logic/converter/convert/core/activity_mapper.hpp
#ifndef CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_
#define CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_

#include <string>
#include <vector>

#include "domain/model/daily_log.hpp"
#include "infrastructure/config/models/converter_config_models.hpp"

class ActivityMapper {
 public:
  explicit ActivityMapper(const ConverterConfig& config);
  auto map_activities(DailyLog& day) -> void;

 private:
  struct TimeRange {
    std::string_view start_hhmm;
    std::string_view end_hhmm;
  };

  const ConverterConfig& config_;
  const std::vector<std::string>& wake_keywords_;

  auto is_wake_event(const RawEvent& raw_event) const -> bool;
  auto map_description(std::string_view description) const -> std::string;
  auto apply_duration_rules(std::string_view mapped_description,
                            int duration_minutes) const -> std::string;
  auto apply_top_parent_mapping(std::vector<std::string>& parts) const -> void;
  static auto build_project_path(const std::vector<std::string>& parts)
      -> std::string;
  auto append_activity(DailyLog& day, const RawEvent& raw_event,
                       const TimeRange& time_range,
                       std::string_view mapped_description) const -> void;

  static auto formatTime(std::string_view time_str_hhmm) -> std::string;
  static auto calculateDurationMinutes(const TimeRange& time_range) -> int;
};

#endif  // CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_
