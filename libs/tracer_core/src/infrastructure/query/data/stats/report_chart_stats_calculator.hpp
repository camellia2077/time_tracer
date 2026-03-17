// infrastructure/query/data/stats/report_chart_stats_calculator.hpp
#pragma once

#include <string_view>
#include <vector>

#include "infrastructure/query/data/data_query_types.hpp"
#include "infrastructure/query/data/stats/stats_models.hpp"

namespace tracer::core::infrastructure::query::data::stats {

struct ReportChartDateRange {
  std::string_view start_date;
  std::string_view end_date;
};

[[nodiscard]] auto BuildReportChartSeries(
    ReportChartDateRange range,
    const std::vector<DayDurationRow>& sparse_rows)
    -> ReportChartSeriesResult;

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::BuildReportChartSeries;

}  // namespace tracer_core::infrastructure::query::data::stats
