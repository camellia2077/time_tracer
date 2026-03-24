#pragma once

#include <string>

namespace tracer::core::infrastructure::query::data::renderers::detail {

inline auto FormatQueryDurationText(long long total_seconds) -> std::string {
  constexpr long long kSecondsInHour = 3600;
  constexpr long long kSecondsInMinute = 60;

  const long long hours = total_seconds / kSecondsInHour;
  const long long minutes = (total_seconds % kSecondsInHour) / kSecondsInMinute;

  std::string output;
  output.reserve(32U);
  output += std::to_string(hours);
  output += "h ";
  output += std::to_string(minutes);
  output += "m";
  return output;
}

}  // namespace tracer::core::infrastructure::query::data::renderers::detail
