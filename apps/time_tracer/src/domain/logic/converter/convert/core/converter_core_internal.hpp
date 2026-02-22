#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "domain/logic/converter/convert/core/converter_core.hpp"

namespace converter_core_internal {

[[nodiscard]] auto NormalizeTime(std::string_view time_str) -> std::string;

[[nodiscard]] auto CalculateWrappedDurationSeconds(std::string_view start_hhmm,
                                                   std::string_view end_hhmm)
    -> std::optional<int>;

[[nodiscard]] auto MergeSpans(const std::optional<SourceSpan>& start_span,
                              const std::optional<SourceSpan>& end_span)
    -> std::optional<SourceSpan>;

class DayStats {
 public:
  static void CalculateStats(DailyLog& day);
};

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

  [[nodiscard]] static auto CalculateDurationMinutes(const TimeRange& range)
      -> int;
};

}  // namespace converter_core_internal
