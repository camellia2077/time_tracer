// infrastructure/query/data/stats/day_duration_stats_calculator.hpp
#pragma once

#include <vector>

#include "infrastructure/query/data/data_query_types.hpp"

namespace tracer_core::infrastructure::query::data::stats {

[[nodiscard]] auto ComputeDayDurationStats(
    const std::vector<DayDurationRow>& rows) -> DayDurationStats;

}  // namespace tracer_core::infrastructure::query::data::stats
