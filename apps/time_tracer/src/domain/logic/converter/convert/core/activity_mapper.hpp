// domain/logic/converter/convert/core/activity_mapper.hpp
#ifndef CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_
#define CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_

#include <optional>
#include <string>
#include <vector>

#include "domain/model/daily_log.hpp"
#include "infrastructure/config/models/converter_config_models.hpp"

class ActivityMapper {
 public:
  explicit ActivityMapper(const ConverterConfig& config);
  auto MapActivities(DailyLog& day) -> void;

 private:
  struct TimeRange {
    std::string_view start_hhmm;
    std::string_view end_hhmm;
  };

  const ConverterConfig& config_;
  const std::vector<std::string>& wake_keywords_;

  [[nodiscard]] auto IsWakeEvent(const RawEvent& raw_event) const -> bool;
  [[nodiscard]] auto MapDescription(std::string_view description) const
      -> std::string;
  [[nodiscard]] auto ApplyDurationRules(std::string_view mapped_description,
                                        int duration_minutes) const
      -> std::string;
  auto ApplyTopParentMapping(std::vector<std::string>& parts) const -> void;
  [[nodiscard]] static auto BuildProjectPath(
      const std::vector<std::string>& parts) -> std::string;
  auto AppendActivity(DailyLog& day, const RawEvent& raw_event,
                      const TimeRange& time_range,
                      std::string_view mapped_description,
                      const std::optional<SourceSpan>& start_span) const
      -> void;

  [[nodiscard]] static auto FormatTime(std::string_view time_str_hhmm)
      -> std::string;
  [[nodiscard]] static auto CalculateDurationMinutes(
      const TimeRange& time_range) -> int;
};

#endif  // CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_H_
