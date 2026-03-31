#ifndef APPLICATION_PIPELINE_DETAIL_PIPELINE_RECORD_TIME_ORDER_SUPPORT_HPP_
#define APPLICATION_PIPELINE_DETAIL_PIPELINE_RECORD_TIME_ORDER_SUPPORT_HPP_

#include <cctype>
#include <optional>
#include <string_view>

#include "domain/types/time_order_mode.hpp"

namespace tracer::core::application::pipeline::record_time_order {

// Logical-day mode uses 06:00 as the day-boundary pivot.
// We do not special-case concrete pairs like 0009/2058; all HHmm values follow
// this generic axis mapping.
inline constexpr int kLogicalDayCutoffMinutes = 6 * 60;
inline constexpr int kMinutesPerDay = 24 * 60;

[[nodiscard]] inline auto TryParseHhmmToMinutes(std::string_view hhmm)
    -> std::optional<int> {
  if (hhmm.size() != 4U) {
    return std::nullopt;
  }
  for (const char ch : hhmm) {
    if (std::isdigit(static_cast<unsigned char>(ch)) == 0) {
      return std::nullopt;
    }
  }

  const int hours =
      (hhmm[0] - '0') * 10 + (hhmm[1] - '0');
  const int minutes =
      (hhmm[2] - '0') * 10 + (hhmm[3] - '0');
  if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
    return std::nullopt;
  }
  return hours * 60 + minutes;
}

[[nodiscard]] inline auto MapMinutesForTimeOrderMode(const int minutes,
                                                     const TimeOrderMode mode)
    -> int {
  // In logical_day_0600 mode, times before 06:00 are projected to the
  // continuation segment (+1440) so strict comparison remains generic.
  if (mode == TimeOrderMode::kLogicalDay0600 &&
      minutes < kLogicalDayCutoffMinutes) {
    return minutes + kMinutesPerDay;
  }
  return minutes;
}

[[nodiscard]] inline auto IsStrictlyAfter(std::string_view candidate_time,
                                          std::string_view baseline_time,
                                          const TimeOrderMode mode) -> bool {
  const auto candidate_minutes = TryParseHhmmToMinutes(candidate_time);
  const auto baseline_minutes = TryParseHhmmToMinutes(baseline_time);
  if (!candidate_minutes.has_value() || !baseline_minutes.has_value()) {
    return false;
  }
  return MapMinutesForTimeOrderMode(*candidate_minutes, mode) >
         MapMinutesForTimeOrderMode(*baseline_minutes, mode);
}

}  // namespace tracer::core::application::pipeline::record_time_order

#endif  // APPLICATION_PIPELINE_DETAIL_PIPELINE_RECORD_TIME_ORDER_SUPPORT_HPP_
