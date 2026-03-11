// infrastructure/query/data/stats/stats_models.hpp
#pragma once

#include <string>
#include <vector>

namespace tracer::core::infrastructure::query::data::stats {

#include "infrastructure/query/data/stats/detail/stats_models_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::ReportChartAggregateStats;
using tracer::core::infrastructure::query::data::stats::ReportChartSeriesPoint;
using tracer::core::infrastructure::query::data::stats::ReportChartSeriesResult;

}  // namespace tracer_core::infrastructure::query::data::stats