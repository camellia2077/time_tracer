// infra/query/data/stats/average_calculator.hpp
#pragma once

#include <cstdint>

#include "application/dto/query_requests.hpp"

namespace tracer::core::infrastructure::query::data::stats {

[[nodiscard]] constexpr auto ResolveAverageDenominator(
    tracer_core::core::dto::InsightsAverageDayBasis average_day_basis,
    int active_days, int range_days) -> int {
  const int denominator =
      average_day_basis ==
              tracer_core::core::dto::InsightsAverageDayBasis::kCalendarDays
          ? range_days
          : active_days;
  return denominator > 0 ? denominator : 0;
}

template <typename TValue>
[[nodiscard]] constexpr auto CalculateAverageOrZero(TValue total,
                                                    std::int64_t denominator)
    -> TValue {
  return denominator > 0 ? total / static_cast<TValue>(denominator) : TValue{};
}

}  // namespace tracer::core::infrastructure::query::data::stats
