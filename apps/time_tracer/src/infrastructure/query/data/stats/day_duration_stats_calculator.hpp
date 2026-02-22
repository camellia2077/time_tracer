#pragma once

#include <vector>

#include "infrastructure/query/data/data_query_types.hpp"

namespace time_tracer::infrastructure::query::data::stats {

[[nodiscard]] auto ComputeDayDurationStats(
    const std::vector<DayDurationRow>& rows) -> DayDurationStats;

}  // namespace time_tracer::infrastructure::query::data::stats
