// infrastructure/query/data/stats/report_chart_stats_calculator.hpp
#pragma once

#include <string_view>
#include <vector>

#include "infrastructure/query/data/data_query_types.hpp"
#include "infrastructure/query/data/stats/stats_models.hpp"

namespace tracer::core::infrastructure::query::data::stats {

#include "infrastructure/query/data/stats/detail/report_chart_stats_calculator_decl.inc"

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::BuildReportChartSeries;

}  // namespace tracer_core::infrastructure::query::data::stats