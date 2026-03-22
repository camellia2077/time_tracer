// infra/query/data/stats/day_duration_stats_calculator.hpp
#pragma once

#include <vector>

#include "infra/query/data/data_query_types.hpp"

namespace tracer::core::infrastructure::query::data::stats {

[[nodiscard]] auto ComputeDayDurationStats(
    const std::vector<DayDurationRow>& rows) -> DayDurationStats;

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::ComputeDayDurationStats;

}  // namespace tracer_core::infrastructure::query::data::stats
