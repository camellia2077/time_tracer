// infrastructure/query/data/stats/day_duration_stats_calculator.hpp
#pragma once

#include <vector>

#include "infrastructure/query/data/data_query_types.hpp"

namespace tracer::core::infrastructure::query::data::stats {

#include "infrastructure/query/data/stats/detail/day_duration_stats_calculator_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::ComputeDayDurationStats;

}  // namespace tracer_core::infrastructure::query::data::stats